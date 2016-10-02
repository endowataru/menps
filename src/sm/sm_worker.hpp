
#pragma once

#include <mgult/generic/basic_worker.hpp>
#include <mgult/generic/thread_local_worker_base.hpp>
#include <mgult/generic/fcontext_worker_base.hpp>
#include <mgult/generic/ult_id_worker_traits_base.hpp>
#include <mgult/generic/default_worker_deque.hpp>
//#include <mgult/generic/locked_worker_deque.hpp>
#include "ult_ptr_ref.hpp"
#include "ptr_worker_deque.hpp"
#include "ult_desc_pool.hpp"

namespace mgult {

typedef mgbase::size_t      worker_rank_t;

struct sm_worker_traits
    : fcontext_worker_traits_base
    , ult_id_worker_traits_base
{
    typedef sm_worker       derived_type;
    typedef ult_ptr_ref     ult_ref_type;
    
    typedef ptr_worker_deque<default_worker_deque>  worker_deque_type;
};

class sm_scheduler;

class sm_worker
    : public basic_worker<sm_worker_traits>
    , public thread_local_worker_base<sm_worker_traits>
    , public fcontext_worker_base
{
public:
    sm_worker(sm_scheduler& sched, const worker_rank_t rank)
        : sched_(sched)
        , rank_(rank)
        { }
    
    ult_ptr_ref allocate_ult()
    {
        return desc_pool_.allocate_ult();
    }
    
    void deallocate_ult(ult_ptr_ref&& th)
    {
        desc_pool_.deallocate_ult(mgbase::move(th));
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
    
    worker_rank_t get_rank() const MGBASE_NOEXCEPT {
        return rank_;
    }
    
    static sm_worker& renew_worker(const ult_id /*id*/) {
        return get_current_worker();
    }
    
    bool finished();
    
    ult_ptr_ref try_steal_from_another();
    
private:
    sm_scheduler& sched_;
    worker_rank_t rank_;
    
    ult_desc_pool desc_pool_;
};

} // namespace mgult

