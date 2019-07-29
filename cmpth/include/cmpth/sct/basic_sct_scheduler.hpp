
#pragma once

#include <cmpth/wss/basic_scheduler.hpp>

namespace cmpth {

template <typename P>
class basic_sct_scheduler
    : public basic_scheduler<P>
{
    CMPTH_DEFINE_DERIVED(P)
    
    using continuation_type = typename P::continuation_type;
    
    using worker_type = typename P::worker_type;
    using worker_num_type = typename worker_type::worker_num_type;
    
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using atomic_bool_type = typename base_ult_itf_type::template atomic<bool>;
    
public:
    basic_sct_scheduler()
    {
        auto& self = this->derived();
        CMPTH_P_ASSERT(P, self_ == nullptr);
        self_ = &self;
        
        worker_type::initialize_tls_global();
    }
    ~basic_sct_scheduler()
    {
        worker_type::finalize_tls_global();
        
        auto& self = this->derived();
        CMPTH_P_ASSERT(P, self_ == &self);
        self_ = nullptr;
    }
    
    bool is_finished() const noexcept {
        return this->finished_.load(fdn::memory_order_relaxed);
    }
    void set_finished() noexcept {
        this->finished_.store(true, fdn::memory_order_release);
    }
    
    continuation_type try_steal_from_others(worker_type& cur_wk)
    {
        const auto num_wks = this->get_num_workers();
        const auto cur_wk_num = cur_wk.get_worker_num();
        
        // TODO: better algorithm
        const auto rand_val = static_cast<worker_num_type>(std::rand());
        
        const auto victim_rank =
            (cur_wk_num + rand_val % (num_wks - 1) + 1) % num_wks;
        
        auto& victim_wk =
            this->get_worker_of_num(victim_rank);
        
        return victim_wk.try_remote_pop_bottom();
    }
    
    static derived_type& get_instance() noexcept {
        CMPTH_P_ASSERT(P, self_ != nullptr);
        return *self_;
    }
    
private:
    atomic_bool_type finished_{false};
    
    static derived_type* self_;
};

template <typename P>
typename basic_sct_scheduler<P>::derived_type*
basic_sct_scheduler<P>::self_ = nullptr;

} // namespace cmpth

