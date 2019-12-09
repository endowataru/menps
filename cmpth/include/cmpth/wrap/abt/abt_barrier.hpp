
#pragma once

#include <cmpth/wrap/abt/abt.hpp>

namespace cmpth {

template <typename P>
class abt_barrier
{
public:
    explicit abt_barrier(fdn::size_t num_waiters) {
        abt_error::check_error(
            ABT_barrier_create(
                static_cast<fdn::uint32_t>(num_waiters)
            ,   &this->bar_
            )
        );
    }
    ~abt_barrier() {
        ABT_barrier_free(&this->bar_);
    }

    abt_barrier(const abt_barrier&) = delete;
    abt_barrier& operator = (const abt_barrier&) = delete;

    void arrive_and_wait() noexcept {
        ABT_barrier_wait(this->bar_);
    }

private:
    ABT_barrier bar_ = ABT_BARRIER_NULL;
};

} // namespace cmpth

