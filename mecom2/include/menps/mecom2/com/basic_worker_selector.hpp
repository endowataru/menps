
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/medev2/ucx/ucp/ucp.hpp>
#include <menps/mefdn/arithmetic.hpp>

#define MECOM2_USE_ULT_WORKER_NUM

namespace menps {
namespace mecom2 {

template <typename P>
class basic_worker_selector
{
    using worker_num_type = typename P::worker_num_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using mutex_type = typename ult_itf_type::mutex;
    using mutex_unique_lock_type = typename ult_itf_type::unique_mutex_lock; // TODO
    
public:
    template <typename Conf>
    explicit basic_worker_selector(const Conf& conf)
        : max_num_(conf.max_wk_num)
    {
        #ifdef MECOM2_USE_ULT_WORKER_NUM
        const auto num_ths = ult_itf_type::get_num_workers();
        this->wk_nums_ = mefdn::make_unique<worker_num_type []>(num_ths);
        
        const auto nums_ths_per_wk = mefdn::roundup_divide(num_ths, this->max_num_);
        MEFDN_ASSERT(nums_ths_per_wk > 0);
        for (worker_num_type i = 0; i < num_ths; ++i) {
            this->wk_nums_[i] = i / nums_ths_per_wk;
            MEFDN_ASSERT(this->wk_nums_[i] < this->max_num_);
        }
        #endif
    }
    
    worker_num_type current_worker_num()
    {
        #ifdef MECOM2_USE_ULT_WORKER_NUM
        return this->wk_nums_[ult_itf_type::get_worker_num()];
        //return ult_itf_type::get_worker_num();
        
        #else
        // Load the TLS.
        auto cur_num = cur_num_;
        
        if (MEFDN_UNLIKELY(cur_num == 0)) {
            mutex_unique_lock_type lk(this->mtx_);
            if (this->alloc_num_ >= this->max_num_) {
                // TODO: Better exception class.
                throw std::bad_alloc();
            }
            
            cur_num = ++this->alloc_num_;
            cur_num_ = cur_num;
        }
        
        MEFDN_ASSERT(cur_num >= 1);
        
        return cur_num - 1;
        #endif
    }
    
private:
    worker_num_type max_num_ = 0;
    
    #ifdef MECOM2_USE_ULT_WORKER_NUM
    mefdn::unique_ptr<worker_num_type []> wk_nums_;
    
    #else
    // Note: Valid numbers start from 1.
    static MEFDN_THREAD_LOCAL worker_num_type cur_num_;
    
    mutex_type mtx_;
    worker_num_type alloc_num_ = 0;
    #endif
};

#ifndef MECOM2_USE_ULT_WORKER_NUM
template <typename P>
MEFDN_THREAD_LOCAL typename P::worker_num_type
basic_worker_selector<P>::cur_num_ = 0;
#endif

} // namespace mecom2
} // namespace menps

