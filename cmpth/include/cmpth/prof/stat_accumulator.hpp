
#pragma once

#include <cmpth/fdn.hpp>
#include <limits>
#include <cmath>
#include <sstream>

namespace cmpth {

template <typename P>
class stat_accumulator
{
    using size_type = fdn::size_t;
    using sample_type = typename P::sample_type;
    using real_type = typename P::real_type;
    
public:
    void add(const sample_type sample) noexcept
    {
        ++this->count_;
        
        this->min_ = std::min(this->min_, sample);
        this->max_ = std::min(this->max_, sample);
        
        const auto rs = static_cast<real_type>(sample);
        this->sum_ += rs;
        this->squared_sum_ += rs*rs;
    }
    
    size_type count() const noexcept { return this->count_; }
    real_type sum() const noexcept { return this->sum_; }
    real_type mean() const noexcept {
        return this->sum() / static_cast<real_type>(this->count());
    }
    real_type variance() const noexcept {
        const auto m = this->mean();
        return this->squared_sum_ - m*m;
    }
    real_type stddev() const noexcept {
        return std::sqrt(this->variance());
    }
    
    sample_type min() const noexcept { return this->min_; }
    sample_type max() const noexcept { return this->max_; }
    
    template <typename OutputStream>
    void print_yaml(OutputStream& os) const {
        os << "{ mean: " << this->mean() <<
            ", std_dev: " << this->stddev() <<
            ", count: " << this->count() <<
            ", sum: " << this->sum() << " }";
    }
    std::string to_yaml() const {
        std::stringstream ss;
        this->print_yaml(ss);
        return ss.str();
    }
    
private:
    size_type count_ = 0;
    real_type sum_ = 0.0;
    real_type squared_sum_ = 0.0;
    sample_type min_ = std::numeric_limits<sample_type>::max();
    sample_type max_ = std::numeric_limits<sample_type>::min();
};

} // namespace cmpth

