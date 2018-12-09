
#pragma once

#include <menps/mecom/rma/command_queue.hpp>

namespace menps {
namespace mecom {
namespace rma {

class offload_requester
    : public rma::requester
{
public:
    typedef command_queue*  queue_ptr_type;
    
    struct config
    {
        mefdn::size_t  num_procs;
        mefdn::size_t  num_ques_per_proc;
    };
    
    explicit offload_requester(const config& conf)
        : conf_(conf)
        , queues_(mefdn::make_unique<mefdn::unique_ptr<queue_ptr_type []> []>(conf_.num_procs))
    {
        for (process_id_t proc = 0; proc < conf.num_procs; ++proc) {
            queues_[proc] = mefdn::make_unique<queue_ptr_type []>(conf_.num_ques_per_proc);
        }
    }
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> async_read(
        const async_untyped_read_params& params
    ) MEFDN_OVERRIDE
    {
        return this->enqueue(command_code::read, params.src_proc, params);
    }
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> async_write(
        const async_untyped_write_params& params
    ) MEFDN_OVERRIDE
    {
        return this->enqueue(command_code::write, params.dest_proc, params);
    }
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> async_atomic_read(
        const async_atomic_read_params<atomic_default_t>& params
    ) MEFDN_OVERRIDE
    {
        return this->enqueue(command_code::atomic_read, params.src_proc, params);
    }
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> async_atomic_write(
        const async_atomic_write_params<atomic_default_t>& params
    ) MEFDN_OVERRIDE
    {
        return this->enqueue(command_code::atomic_write, params.dest_proc, params);
    }
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> async_compare_and_swap(
        const async_compare_and_swap_params<atomic_default_t>& params
    ) MEFDN_OVERRIDE
    {
        return this->enqueue(command_code::compare_and_swap, params.target_proc, params);
    }
    
    MEFDN_NODISCARD
    virtual ult::async_status<void> async_fetch_and_add(
        const async_fetch_and_add_params<atomic_default_t>& params
    ) MEFDN_OVERRIDE
    {
        return this->enqueue(command_code::fetch_and_add, params.target_proc, params);
    }
    
protected:
    void set_command_queue(
        process_id_t    proc
    ,   mefdn::size_t  que_index
    ,   queue_ptr_type  que
    ) {
        queues_[proc][que_index] = mefdn::move(que);
    }
    
private:
    template <typename Params>
    ult::async_status<void> enqueue(
        const command_code  code
    ,   const process_id_t  proc
    ,   const Params&       params
    ) {
        MEFDN_ASSERT(valid_process_id(proc));
        
        auto que_indexes = que_indexes_;
        if (MEFDN_UNLIKELY(que_indexes == nullptr)) {
            const auto num_procs = conf_.num_procs;
            
            que_indexes_ = que_indexes = new mefdn::size_t[num_procs]; // value-initialization
            // TODO : memory leak
            
            std::uninitialized_fill_n(que_indexes, num_procs, 0);
        }
        
        const auto que_index = que_indexes[proc];
        MEFDN_ASSERT(que_index < conf_.num_ques_per_proc);
        
        auto& que = *queues_[proc][que_index];
        
        //if (que.peek_num_entries() > 1024) {
            if (++que_indexes[proc] >= conf_.num_ques_per_proc) {
                que_indexes[proc] = 0;
            }
        //}
        
        while (true)
        {
            auto t = que.try_enqueue(1, true);
            
            if (MEFDN_LIKELY(t.valid()))
            {
                auto& dest = *t.begin();
                
                dest.code = code;
                
                MEFDN_STATIC_ASSERT(sizeof(Params) <= command::args_size);
                reinterpret_cast<Params&>(dest.arg) = params;
                
                t.commit(1);
                
                que.notify_if_sleeping(t);
                
                return ult::make_async_deferred<void>();
            }
            
            ult::this_thread::yield();
        }
    }
    
    const config conf_;
    
    mefdn::unique_ptr<mefdn::unique_ptr<queue_ptr_type []> []> queues_;
    
    static MEFDN_THREAD_LOCAL mefdn::size_t* que_indexes_;
};

} // namespace rma
} // namespace mecom
} // namespace menps

