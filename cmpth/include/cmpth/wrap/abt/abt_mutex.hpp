
#pragma once

#include <cmpth/wrap/abt/abt.hpp>

namespace cmpth {

template <typename P>
class abt_mutex
{
public:
    abt_mutex() {
        abt_error::check_error(
            ABT_mutex_create(&this->mtx_)
        );
    }
    ~abt_mutex() /*noexcept*/ {
        ABT_mutex_free(&this->mtx_);
    }

    abt_mutex(const abt_mutex&) = delete;
    abt_mutex& operator = (const abt_mutex&) = delete;

    void lock() {
        abt_error::check_error(
            ABT_mutex_lock(this->mtx_)
        );
    }
    void unlock() {
        abt_error::check_error(
            ABT_mutex_unlock(this->mtx_)
        );
    }

private:
    ABT_mutex mtx_ = ABT_MUTEX_NULL;
};

} // namespace cmpth
