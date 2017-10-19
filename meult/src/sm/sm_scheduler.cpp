
#include <menps/meult/sm/impl/sm_scheduler.hpp>

namespace menps {
namespace meult {
namespace sm {

namespace /*unnamed*/ {

worker_rank_t get_num_ranks_from_env()
{
    const auto s = getenv("MEULT_NUM_WORKERS");
    
    if (s != nullptr) {
        auto num_ranks = static_cast<worker_rank_t>(atoi(s));
        if (num_ranks == 0) {
            // TODO: Should it be an exception?
            return 1; // Default
        }
        return num_ranks;
    }
    else
        return 1; // Default
}

} // unnamed namespace

sm_scheduler::sm_scheduler()
{
    instance_ = this;
    
    num_ranks_ = get_num_ranks_from_env();
}

ult_ptr_ref sm_scheduler::try_steal_from_another(sm_worker& w)
{
    MEFDN_ASSERT(&sm_worker::get_current_worker() == &w);
    
    const auto num_ranks = get_num_ranks();
    
    if (num_ranks == 1)
        return ult_ptr_ref{};
    
    const auto current_rank = w.get_rank();
    
    // TODO: better algorithm
    const auto random_val = static_cast<worker_rank_t>(std::rand());
    
    const auto stolen_rank = (current_rank + random_val % (num_ranks - 1) + 1) % num_ranks;
    
    MEFDN_ASSERT(stolen_rank != current_rank);
    MEFDN_ASSERT(stolen_rank < num_ranks);
    
    auto& stolen_wk = this->get_worker_of_rank(stolen_rank);
    
    return stolen_wk.try_steal();
}

sm_scheduler* sm_scheduler::instance_; // TODO


bool sm_worker::finished()
{
    return sched_.finished();
}

ult_ptr_ref sm_worker::try_steal_from_another()
{
    return sched_.try_steal_from_another(*this);
}

} // namespace sm
} // namespace meult
} // namespace menps

