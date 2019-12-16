
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_sct_initializer
{
    using scheduler_type = typename P::scheduler_type;
    using initializer_type = typename scheduler_type::initializer;

public:
    basic_sct_initializer() {
        const char* const num_wks_str = std::getenv("CMPTH_NUM_WORKERS");
        int num_wks = num_wks_str ? std::atoi(num_wks_str) : 1;
        this->sched_.make_workers(num_wks);
        
        this->init_ = fdn::make_unique<initializer_type>(this->sched_);
    }
    explicit basic_sct_initializer(int /*argc*/, char** /*argv*/)
        : basic_sct_initializer{}
    { }
    
    basic_sct_initializer(const basic_sct_initializer&) = delete;
    basic_sct_initializer& operator = (const basic_sct_initializer&) = delete;

private:
    scheduler_type sched_;
    fdn::unique_ptr<initializer_type> init_;
};

} // namespace cmpth

