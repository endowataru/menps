
#include "global_ult_desc_pool.hpp"

#include <menps/mecom/structure/alltoall_buffer.hpp>
#include <menps/mefdn/container/circular_buffer.hpp>

#include <menps/mecom/rpc.hpp>

#include <menps/mefdn/memory/next_in_bytes.hpp>

namespace menps {
namespace meth {

class global_ult_desc_pool_error
    : public std::exception { };

class global_ult_desc_pool::impl
{
    static const mefdn::size_t num_descs = 1 << 8;
    
    typedef mefdn::mutex                   mutex_type;
    typedef mefdn::condition_variable      cv_type;
    typedef mefdn::unique_lock<mutex_type> unique_lock_type;
    
public:
    explicit impl(const config& conf)
        : conf_(conf)
        , local_descs_{ mecom::rma::allocate<global_ult_desc>(num_descs) }
    {
        mecom::rpc::register_handler2<deallocate_handler>(
            mecom::rpc::requester::get_instance()
        ,   deallocate_handler{}
        );
        
        for (mefdn::size_t i = 0; i < num_descs; ++i)
        {
            auto& desc = local_descs_[i];
            
            // Allocate a stack region.
            const auto stack_first_ptr
                = mefdn::next_in_bytes(
                    conf.stack_segment_ptr
                ,   i * stack_size
                );
            
            const auto stack_last_ptr
                = static_cast<mefdn::uint8_t*>(stack_first_ptr) + stack_size;
            
            desc.stack_ptr = stack_last_ptr;
            desc.stack_size = stack_size;
            
            MEFDN_LOG_VERBOSE(
                "msg:Prepare thread descriptor.\t"
                "i:{}\t"
                "stack_ptr:0x{:x}\t"
                //"stack_size:0x{:x}"
            ,   i
            ,   reinterpret_cast<mefdn::uintptr_t>(stack_last_ptr)
            //,   stack_size
            );
        }
        
        bufs_.collective_initialize(local_descs_);
        
        indexes_.set_capacity(num_descs);
        
        for (mefdn::size_t i = 0; i < num_descs; ++i)
            indexes_.push_back(i);
        
        instance_ = this;
        
        MEFDN_LOG_VERBOSE("msg:Initialized global ult desc pool.");
    }
    
    ~impl()
    {
        MEFDN_LOG_VERBOSE("msg:Finalizing global ult desc pool.");
        
        bufs_.finalize();
        mecom::rma::deallocate(local_descs_);
    }
    
    global_ult_ref allocate_ult()
    {
        const auto index = this->allocate_ult_index();
        
        auto& desc = local_descs_[index];
        
        // Initialize the thread descriptor.
        desc.owner = -1;
        desc.state = global_ult_state::ready;
        desc.joiner = meult::make_invalid_ult_id();
        #ifdef METH_ENABLE_ASYNC_WRITE_BACK
        desc.cur_stamp = 0;
        desc.old_stamp = 0;
        #endif
        desc.detached = false;
        
        MEFDN_ASSERT(index < num_descs);
        
        const auto current_proc = mecom::current_process_id();
        
        ult_id id;
        id.di.proc = current_proc;
        id.di.local_id = index;
        
        return get_ult_ref_from_id(id);
    }
    
private:
    mefdn::size_t allocate_ult_index()
    {
        unique_lock_type lk(this->mtx_);
        
        while (this->indexes_.empty()) {
            cv_.wait(lk);
        }
        
        const auto index = this->indexes_.front();
        this->indexes_.pop_front();
        
        return index;
    }
    
public:
    global_ult_ref get_ult_ref_from_id(const ult_id& id)
    {
        auto desc_rptr_first = bufs_.at_process(id.di.proc);
        
        const auto desc_rptr = desc_rptr_first + id.di.local_id;
        
        return { id, desc_rptr };
    }
    
    void deallocate_ult(global_ult_ref&& th)
    {
        const auto th_id = th.get_id();
        
        MEFDN_LOG_INFO(
            "msg:Deallocate thread descriptor.\t"
            "{}"
        ,   th.to_string()
        );
        
        if (th_id.di.proc != mecom::current_process_id()) {
            // Write back the memory to flush the call stack.
            // The call stack will no longer be used again,
            // but local modifications must be applied before the revival.
            conf_.space.write_barrier();
        }
        
        // Invalidate the descriptor (for debugging).
        th.invalidate_desc();
        
        const auto proc = th_id.di.proc;
        
        mecom::rpc::call<deallocate_handler>(
            mecom::rpc::requester::get_instance()
        ,   proc
        ,   th_id
        );
    }
    
private:
    struct deallocate_handler
    {
        static const mecom::rpc::handler_id_t handler_id = 1010;
        
        typedef ult_id      request_type;
        typedef void        reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& rqst = sc.request();
            
            instance_->deallocate_on_this_process(rqst);
            
            return sc.make_reply();
        }
    };
    
    void deallocate_on_this_process(const ult_id& id)
    {
        auto& di = id.di;
        
        MEFDN_ASSERT(di.proc == mecom::current_process_id());
        MEFDN_ASSERT(di.local_id < num_descs);
        
        unique_lock_type lk(this->mtx_);
        
        // Return the ID.
        indexes_.push_front(di.local_id); // LIFO
        //indexes_.push_back(di.local_id); // FIFO
        
        cv_.notify_one();
    }
    
    const config conf_;
    
    mecom::rma::local_ptr<global_ult_desc> local_descs_;
    
    mecom::structure::alltoall_buffer<global_ult_desc> bufs_;
    
    mutex_type mtx_;
    cv_type cv_;
    mefdn::circular_buffer<mefdn::size_t> indexes_;
    
    static impl* instance_; // TODO
};

global_ult_desc_pool::impl* global_ult_desc_pool::impl::instance_ = nullptr; // TODO

global_ult_desc_pool::global_ult_desc_pool(const config& conf)
    : impl_{new impl{conf}}
    {}

global_ult_desc_pool::~global_ult_desc_pool() = default;

global_ult_ref global_ult_desc_pool::allocate_ult() {
    return impl_->allocate_ult();
}

void global_ult_desc_pool::deallocate_ult(global_ult_ref&& th) {
    impl_->deallocate_ult(mefdn::move(th));
}

global_ult_ref global_ult_desc_pool::get_ult_ref_from_id(const ult_id& id) {
    return impl_->get_ult_ref_from_id(id);
}

} // namespace meth
} // namespace menps

