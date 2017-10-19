
#pragma once

#include <menps/mefdn/lang.hpp>
#include <limits>
#include <sstream>
#include <cmath>

namespace menps {
namespace mefdn {

template <typename Sample, typename Real = double>
class average_accumulator {
public:
    average_accumulator() noexcept
        : count_(0), sum_(0.0), squared_sum_(0.0)
        , min_(std::numeric_limits<Sample>::min())
        , max_(std::numeric_limits<Sample>::max()) { }
    
    average_accumulator(const average_accumulator&) noexcept = default;
    average_accumulator& operator = (const average_accumulator&) noexcept = default;
    
    void add(Sample sample) noexcept {
        ++count_;
        
        min_ = std::min(min_, sample);
        max_ = std::max(max_, sample);
        
        const Real real_sample = static_cast<Real>(sample);
        sum_ += real_sample;
        squared_sum_ += real_sample * real_sample;
    }
    
    std::size_t count() const noexcept { return count_; }
    
    Real sum() const noexcept { return sum_; }
    
    Real mean() const noexcept {
        return sum() / static_cast<Real>(count());
    }
    Real stddev() const noexcept {
        return std::sqrt(variance());
    }
    Real variance() const noexcept {
        const Real mean_val = mean();
        return squared_sum_ - mean_val*mean_val;
    }
    
    Sample min() const noexcept { return min_; }
    Sample max() const noexcept { return max_; }
    
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

} // namespace mefdn
} // namespace menps

