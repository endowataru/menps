
#include "global_ult_desc_pool.hpp"

#include <mgcom/structure/alltoall_buffer.hpp>
#include <mgbase/container/circular_buffer.hpp>

#include <mgcom/rpc.hpp>
#include <mgcom/rpc/call2.hpp>

#include <mgbase/memory/next_in_bytes.hpp>

namespace mgth {

class global_ult_desc_pool_error
    : public std::exception { };

class global_ult_desc_pool::impl
{
    static const mgbase::size_t num_descs = 1 << 8;
    
    typedef mgbase::mutex                   mutex_type;
    typedef mgbase::condition_variable      cv_type;
    typedef mgbase::unique_lock<mutex_type> unique_lock_type;
    
public:
    explicit impl(const config& conf)
        : local_descs_{ mgcom::rma::allocate<global_ult_desc>(num_descs) }
    {
        mgcom::rpc::register_handler2<deallocate_handler>(
            mgcom::rpc::requester::get_instance()
        ,   deallocate_handler{}
        );
        
        for (mgbase::size_t i = 0; i < num_descs; ++i)
        {
            auto& desc = local_descs_[i];
            
            // Allocate a stack region.
            const auto stack_first_ptr
                = mgbase::next_in_bytes(
                    conf.stack_segment_ptr
                ,   i * stack_size
                );
            
            const auto stack_last_ptr
                = static_cast<mgbase::uint8_t*>(stack_first_ptr) + stack_size;
            
            desc.stack_ptr = stack_last_ptr;
            desc.stack_size = stack_size;
            
            MGBASE_LOG_VERBOSE(
                "msg:Prepare thread descriptor.\t"
                "i:{}\t"
                "stack_ptr:0x{:x}\t"
                //"stack_size:0x{:x}"
            ,   i
            ,   reinterpret_cast<mgbase::uintptr_t>(stack_last_ptr)
            //,   stack_size
            );
        }
        
        bufs_.collective_initialize(local_descs_);
        
        indexes_.set_capacity(num_descs);
        
        for (mgbase::size_t i = 0; i < num_descs; ++i)
            indexes_.push_back(i);
        
        instance_ = this;
        
        MGBASE_LOG_VERBOSE("msg:Initialized global ult desc pool.");
    }
    
    ~impl()
    {
        MGBASE_LOG_VERBOSE("msg:Finalizing global ult desc pool.");
        
        bufs_.finalize();
        mgcom::rma::deallocate(local_descs_);
    }
    
    global_ult_ref allocate_ult()
    {
        const auto index = this->allocate_ult_index();
        
        auto& desc = local_descs_[index];
        
        // Initialize the thread descriptor.
        desc.owner = -1;
        desc.ctx = context_t{};
        desc.state = global_ult_state::ready;
        desc.joiner = mgult::make_invalid_ult_id();
        
        MGBASE_ASSERT(index < num_descs);
        
        const auto current_proc = mgcom::current_process_id();
        
        ult_id id;
        id.di.proc = current_proc;
        id.di.local_id = index;
        
        return get_ult_ref_from_id(id);
    }
    
private:
    mgbase::size_t allocate_ult_index()
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
        
        const auto proc = th_id.di.proc;
        
        mgcom::rpc::call2<deallocate_handler>(
            mgcom::rpc::requester::get_instance()
        ,   proc
        ,   th_id
        );
    }
    
private:
    struct deallocate_handler
    {
        static const mgcom::rpc::handler_id_t handler_id = 1010;
        
        typedef ult_id      request_type;
        typedef void        reply_type;
        
        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc)
        {
            auto& rqst = sc.request();
            
            instance_->deallocate_on_this_process(rqst);
            
            return sc.make_reply();
        }
    };
    
    void deallocate_on_this_process(const ult_id& id)
    {
        auto& di = id.di;
        
        MGBASE_ASSERT(di.proc == mgcom::current_process_id());
        
        
        unique_lock_type lk(this->mtx_);
        
        // Return the ID.
        indexes_.push_back(di.local_id);
        
        cv_.notify_one();
    }
    
    mgcom::rma::local_ptr<global_ult_desc> local_descs_;
    
    mgcom::structure::alltoall_buffer<global_ult_desc> bufs_;
    
    mutex_type mtx_;
    cv_type cv_;
    mgbase::circular_buffer<mgbase::size_t> indexes_;
    
    static impl* instance_; // TODO
};

global_ult_desc_pool::impl* global_ult_desc_pool::impl::instance_ = MGBASE_NULLPTR; // TODO

global_ult_desc_pool::global_ult_desc_pool(const config& conf)
    : impl_{new impl{conf}}
    {}

global_ult_desc_pool::~global_ult_desc_pool() = default;

global_ult_ref global_ult_desc_pool::allocate_ult() {
    return impl_->allocate_ult();
}

void global_ult_desc_pool::deallocate_ult(global_ult_ref&& th) {
    impl_->deallocate_ult(mgbase::move(th));
}

global_ult_ref global_ult_desc_pool::get_ult_ref_from_id(const ult_id& id) {
    return impl_->get_ult_ref_from_id(id);
}

} // namespace mgth

