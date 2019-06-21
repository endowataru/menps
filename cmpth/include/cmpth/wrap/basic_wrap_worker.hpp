
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_wrap_worker
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_num_type = typename P::worker_num_type;
    
public:
    worker_num_type get_worker_num() const noexcept {
        return this->wk_num_;
    }
    void set_worker_num(const worker_num_type wk_num) noexcept {
        this->wk_num_ = wk_num;
    }
    
    static basic_wrap_worker& get_cur_worker() noexcept;
    
private:
    worker_num_type wk_num_ = 0;
};

template <typename P>
class basic_wrap_scheduler
{
    using base_ult_itf_type = typename P::base_ult_itf_type;
    using worker_num_type = typename P::worker_num_type;
    using worker_type = basic_wrap_worker<P>;
    
public:
    basic_wrap_scheduler()
    {
        const auto n_wks = base_ult_itf_type::get_num_workers();
        this->wks_ = fdn::make_unique<worker_type []>(n_wks);
        for (worker_num_type wk_num = 0; wk_num < n_wks; ++wk_num) {
            this->wks_[wk_num].set_worker_num(wk_num);
        }
    }
    
    worker_type& get_worker_at(worker_num_type wk_num) noexcept {
        CMPTH_P_ASSERT(P, wk_num < base_ult_itf_type::get_num_workers());
        return this->wks_[wk_num];
    }
    
    static basic_wrap_scheduler& get_instance() noexcept {
        static basic_wrap_scheduler sched;
        return sched;
    }
    
private:
    fdn::unique_ptr<worker_type []> wks_;
};

template <typename P>
basic_wrap_worker<P>& basic_wrap_worker<P>::get_cur_worker() noexcept {
    auto& sched = basic_wrap_scheduler<P>::get_instance();
    const auto wk_num = base_ult_itf_type::get_worker_num();
    return sched.get_worker_at(wk_num);
}

template <typename P>
struct basic_wrap_worker_itf
{
    using worker = basic_wrap_worker<P>;
    using scheduler = basic_wrap_scheduler<P>;
};

} // namespace cmpth

