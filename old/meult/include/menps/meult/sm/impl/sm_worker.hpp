
#pragma once

#include <menps/meult/generic/basic_worker.hpp>
#include <menps/meult/generic/basic_current_thread.hpp>
#include <menps/meult/generic/thread_local_worker_base.hpp>
#include <menps/meult/generic/ult_id_worker_traits_base.hpp>
#include <menps/meult/generic/default_worker_deque.hpp>
#include "ult_ptr_ref.hpp"
#include "ptr_worker_deque.hpp"
#include "ult_desc_pool.hpp"
#include <menps/mectx/context_policy.hpp>

namespace menps {
namespace meult {
namespace sm {

typedef mefdn::size_t      worker_rank_t;

struct sm_worker_traits
    : ult_id_worker_traits_base
{
    typedef sm_worker       derived_type;
    typedef ult_ptr_ref     ult_ref_type;
    
    typedef ptr_worker_deque<default_worker_deque>  worker_deque_type;
    
    typedef ptr_worker_deque_conf worker_deque_conf_type;
    
    typedef mectx::context<sm_worker*>  context_type;
    typedef mectx::transfer<sm_worker*> transfer_type;
    
    struct allocated_ult_type
    {
        ult_id_type id;
        void*       ptr;
    };
};

class sm_scheduler;

class sm_worker
    : public basic_worker<sm_worker_traits>
    , public basic_current_thread<sm_worker_traits>
    , public thread_local_worker_base<sm_worker_traits>
    , public mectx::context_policy
{
    typedef basic_worker<sm_worker_traits>  base;
    
public:
    sm_worker(sm_scheduler& sched, const worker_rank_t rank)
        : base({1024 * 1024})
        , sched_(sched)
        , rank_(rank)
        { }
    
    ult_ptr_ref allocate_ult()
    {
        return desc_pool_.allocate_ult();
    }
    
    void deallocate_ult(ult_ptr_ref&& th)
    {
        desc_pool_.deallocate_ult(mefdn::move(th));
    }
    void deallocate_ult_on_sm(ult_ptr_ref&& th)
    {
        deallocate_ult(mefdn::move(th));
    }
    
    ult_ptr_ref get_ult_ref_from_id(const ult_id& id)
    {
        return ult_ptr_ref(id);
    }
    
    std::string show_ult_ref(ult_ptr_ref& th)
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
    
    worker_rank_t get_rank() const noexcept {
        return rank_;
    }
    
    static sm_worker& renew_worker(const ult_id /*id*/) {
        return get_current_worker();
    }
    
    void on_before_switch(ult_ptr_ref& /*from_th*/, ult_ptr_ref& /*to_th*/)
    {
        // Do nothing.
        // This hook is for distributed work-stealing.
    }
    
    template <bool IsPrevLocked>
    void on_after_switch(ult_ptr_ref& /*from_th*/, ult_ptr_ref& /*to_th*/)
    {
        // Do nothing.
        // This hook is for distributed work-stealing.
    }
    void on_join_already(ult_ptr_ref& /*current_th*/, ult_ptr_ref& /*joinee_th*/, ult_ptr_ref::unique_lock_type& /*lk*/) {
        // Do nothing.
        // This hook is for distributed work-stealing.
    }
    void on_join_resume(ult_ptr_ref&& child_th) {
        // Always destroy the child thread without locking.
        this->deallocate_ult( mefdn::move(child_th) );
    }
    void on_exit_resume(ult_ptr_ref& /*th*/) {
        // Do nothing.
        // This hook is for distributed work-stealing.
    }
    
    bool finished();
    
    ult_ptr_ref try_steal_from_another();
    
private:
    sm_scheduler& sched_;
    worker_rank_t rank_;
    
    ult_desc_pool desc_pool_;
};

} // namespace sm
} // namespace meult
} // namespace menps

