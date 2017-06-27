
#pragma once

#include "mth.hpp"

namespace mgult {
namespace backend {
namespace mth {

struct tls_error : std::exception { };

template <typename Policy>
class thread_specific
{
    typedef typename Policy::value_type value_type;
    
public:
    thread_specific() {
        if (myth_key_create(&this->key_, MGBASE_NULLPTR) != 0)
            throw tls_error();
    }
    
    ~thread_specific() /*noexcept*/ {
        myth_key_delete(this->key_);
    }
    
    thread_specific(const thread_specific&) = delete;
    thread_specific& operator = (const thread_specific&) = delete;
    
    value_type* get() {
        return static_cast<value_type*>(
            myth_getspecific(this->key_)
        );
    }
    
    void set(value_type* const ptr) const {
        if (myth_setspecific(this->key_, ptr) != 0)
            throw tls_error();
    }
    
private:
    myth_key_t key_;
};

} // namespace mth
} // namespace backend
} // namespace mgult

