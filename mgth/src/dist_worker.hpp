
#pragma once

#include "global_ult_ref.hpp"
#include "dist_worker_deque.hpp"

#include <mgult/generic/basic_worker.hpp>
#include <mgult/generic/default_worker_deque.hpp>
#include <mgult/generic/thread_local_worker_base.hpp>
#include <mgult/generic/fcontext_worker_base.hpp>
#include <mgult/generic/ult_id_worker_traits_base.hpp>

namespace mgth {

class dist_worker;

struct dist_worker_traits
    : mgult::fcontext_worker_traits_base
    , mgult::ult_id_worker_traits_base
{
    typedef dist_worker                 derived_type;
    typedef global_ult_ref              ult_ref_type;
    typedef dist_worker_deque           worker_deque_type;
    typedef dist_worker_deque_conf      worker_deque_conf_type;
};

class dist_scheduler;

class dist_worker
    : public mgult::basic_worker<dist_worker_traits>
    , public mgult::thread_local_worker_base<dist_worker_traits>
    , public mgult::fcontext_worker_base
{
    typedef mgult::basic_worker<dist_worker_traits> base;
    
public:
    dist_worker(dist_scheduler& sched, const worker_rank_t rank);
    
    // Methods required by basic_worker.
    
    inline global_ult_ref allocate_ult();
    
    inline void deallocate_ult(global_ult_ref&& th);
    
    inline global_ult_ref get_ult_ref_from_id(const mgult::ult_id& id);
    
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
    
    inline bool finished();
    
    global_ult_ref try_steal_from_another() {
        return global_ult_ref{}; // TODO
    }
    
    static dist_worker& renew_worker(const mgult::ult_id& /*id*/) {
        return get_current_worker();
    }
    
    // Other methods.
    
    worker_rank_t get_rank() const MGBASE_NOEXCEPT {
        return rank_;
    }
    
private:
    dist_scheduler&         sched_;
    const worker_rank_t     rank_;
    
    //global_ult_desc_pool    desc_pool_;
};

} // namespace mgth

