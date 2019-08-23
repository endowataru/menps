
#pragma once

#include <cmpth/wrap/mth/mth.hpp>

namespace cmpth {

template <typename P>
class mth_thread_specific
{
    using value_type = typename P::value_type;
    
public:
    mth_thread_specific() {
        if (myth_key_create(&this->key_, nullptr) != 0)
            throw mth_error{};
    }
    
    ~mth_thread_specific() /*noexcept*/ {
        myth_key_delete(this->key_);
    }
    
    mth_thread_specific(const mth_thread_specific&) = delete;
    mth_thread_specific& operator = (const mth_thread_specific&) = delete;
    
    value_type* get() {
        return static_cast<value_type*>(
            myth_getspecific(this->key_)
        );
    }
    
    void set(value_type* const ptr) const {
        if (myth_setspecific(this->key_, ptr) != 0)
            throw mth_error{};
    }
    
private:
    myth_key_t key_;
};

} // namespace cmpth

