
#pragma once

#include "mth.hpp"

namespace menps {
namespace meult {
namespace backend {
namespace mth {

struct mutex_error : std::exception { };

class mutex
{
public:
    typedef myth_mutex_t*   native_handle_type;
    
    mutex() noexcept
    {
        // TODO: On old GCC (4.4), we cannot use {} on member initialization (?).
        myth_mutex_t m = MYTH_MUTEX_INITIALIZER;
        m_ = m;
    }
    
    mutex(const mutex&) = delete;
    mutex& operator = (const mutex&) = delete;
    
    ~mutex() /*noexcept*/
    {
        myth_mutex_destroy(&m_);
        // Ignore the error.
    }
    
    bool try_lock()
    {
        const int ret = myth_mutex_trylock(&m_);
        
        if (ret == 0)
            return true;
        else if (ret == EBUSY)
            return false;
        else
            throw mutex_error();
    }
    
    void lock()
    {
        if (myth_mutex_lock(&m_) != 0)
            throw mutex_error();
    }
    
    
    void unlock()
    {
        if (myth_mutex_unlock(&m_) != 0)
            throw mutex_error();
    }
    
    native_handle_type native_handle() noexcept {
        return &m_;
    }
    
private:
    myth_mutex_t m_;
};

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

