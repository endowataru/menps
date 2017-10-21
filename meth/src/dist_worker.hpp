
#pragma once

#include <menps/meth/common.hpp>
#include "global_ult_ref.hpp"
#include "dist_worker_deque.hpp"

#include <menps/meult/generic/basic_worker.hpp>
#include <menps/meult/generic/basic_current_thread.hpp>
#include <menps/meult/generic/default_worker_deque.hpp>
//#include <menps/meult/generic/thread_local_worker_base.hpp>
#include <menps/meult/generic/thread_specific_worker_base.hpp>
#include <menps/meult/generic/fcontext_worker_base.hpp>
#include <menps/meult/generic/ult_id_worker_traits_base.hpp>
#include <menps/mectx/context_policy.hpp>

#include "alternate_signal_stack.hpp"

namespace menps {
namespace meth {

class dist_worker;

struct dist_worker_traits
    : meult::ult_id_worker_traits_base
    , meth::base_ult::ult_policy
{
    typedef dist_worker                 derived_type;
    typedef global_ult_ref              ult_ref_type;
    typedef dist_worker_deque           worker_deque_type;
    typedef dist_worker_deque_conf      worker_deque_conf_type;
    
    typedef mectx::context<derived_type*>   context_type;
    typedef mectx::transfer<derived_type*>  transfer_type;
    
    struct allocated_ult_type
    {
        ult_id_type id;
        void*       ptr;
    };
};

class dist_scheduler;

class dist_worker
    : public meult::basic_worker<dist_worker_traits>
    , public meult::basic_current_thread<dist_worker_traits>
    //, public meult::thread_local_worker_base<dist_worker_traits>
    , public meult::thread_specific_worker_base<dist_worker_traits>
    , public mectx::context_policy
{
    typedef meult::basic_worker<dist_worker_traits>         base;
    
    //typedef meult::thread_local_worker_base<dist_worker_traits> tls_base;
    typedef meult::thread_specific_worker_base<dist_worker_traits> tls_base;
    
    static const mefdn::size_t join_stack_size = 2 << 20;
    
    static const mefdn::size_t page_fault_stack_size = 2 << 20;
    
public:
    dist_worker(dist_scheduler& sched, const worker_rank_t rank);
    
    // Methods required by basic_worker.
    
    inline global_ult_ref allocate_ult();
    
    inline void deallocate_ult(global_ult_ref&& th);
    
    void deallocate_ult_on_sm(global_ult_ref&& th) {
        // do nothing; deallocation is delayed until the write back is completed
    }
    
    inline global_ult_ref get_ult_ref_from_id(const meult::ult_id& id);
    
    std::string show_ult_ref(global_ult_ref& th)
    {
        fmt::MemoryWriter w;
        w.write(
            "rank:{}\t"
            "{}"
        ,   get_rank()
        ,   th.to_string()
        );
        
        return w.str();
    }
    
    void on_before_switch(global_ult_ref&, global_ult_ref&);
    template <bool IsFromLocked>
    void on_after_switch(global_ult_ref&, global_ult_ref&);
    void on_join_already(global_ult_ref&, global_ult_ref&, global_ult_ref::unique_lock_type&);
    void on_join_resume(global_ult_ref&&);
    void on_exit_resume(global_ult_ref&);
    
    inline bool finished();
    
    global_ult_ref try_steal_from_another();
    
    static dist_worker& renew_worker(const meult::ult_id& /*id*/) {
        return get_current_worker();
    }
    
    void initialize_on_this_thread();
    
    void finalize_on_this_thread();
    
    // Other methods.
    
    worker_rank_t get_rank() const noexcept {
        return rank_;
    }
    
private:
    struct join_already_data;
    
    dist_scheduler&         sched_;
    const worker_rank_t     rank_;
    
    mefdn::unique_ptr<mefdn::uint8_t []>      stack_area_;
    mefdn::unique_ptr<alternate_signal_stack>  alter_stack_;
    
    mefdn::unique_ptr<mefdn::uint8_t []> join_stack_area_;
    
    //global_ult_desc_pool    desc_pool_;
};

} // namespace meth
} // namespace menps

