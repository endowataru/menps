
#pragma once

#include <mgbase/lang.hpp>
#include <limits>
#include <sstream>
#include <cmath>

namespace mgbase {

template <typename Sample, typename Real = double>
class average_accumulator {
public:
    average_accumulator() MGBASE_NOEXCEPT
        : count_(0), sum_(0.0), squared_sum_(0.0)
        , min_(std::numeric_limits<Sample>::min())
        , max_(std::numeric_limits<Sample>::max()) { }
    
    #ifdef MGBASE_CXX11_SUPPORTED
    average_accumulator(const average_accumulator&) noexcept = default;
    
    average_accumulator& operator = (const average_accumulator&) noexcept = default;
    #endif
    
    void add(Sample sample) MGBASE_NOEXCEPT {
        ++count_;
        
        min_ = std::min(min_, sample);
        max_ = std::max(max_, sample);
        
        const Real real_sample = static_cast<Real>(sample);
        sum_ += real_sample;
        squared_sum_ += real_sample * real_sample;
    }
    
    std::size_t count() const MGBASE_NOEXCEPT { return count_; }
    
    Real sum() const MGBASE_NOEXCEPT { return sum_; }
    
    Real mean() const MGBASE_NOEXCEPT {
        return sum() / static_cast<Real>(count());
    }
    Real stddev() const MGBASE_NOEXCEPT {
        return std::sqrt(variance());
    }
    Real variance() const MGBASE_NOEXCEPT {
        const Real mean_val = mean();
        return squared_sum_ - mean_val*mean_val;
    }
    
    Sample min() const MGBASE_NOEXCEPT { return min_; }
    Sample max() const MGBASE_NOEXCEPT { return max_; }
    
    std::string summary() const {
        std::stringstream ss;
        ss << "{ mean: " << mean() << ", std_dev: " << stddev() << ", count: " << count() << ", sum: " << sum() << " }";
        return ss.str();
    }
    
private:
    std::size_t count_;
    Real sum_, squared_sum_;
    Sample min_, max_;
};

template <typename Sample, typename Real>
inline std::ostream& operator << (std::ostream& ost, const average_accumulator<Sample, Real>& value) {
    return ost << value.summary();
}

} // namespace mgbase

