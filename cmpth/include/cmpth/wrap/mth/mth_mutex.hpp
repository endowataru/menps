
#pragma once

#include <cmpth/wrap/mth/mth_itf.hpp>

namespace cmpth {

class mth_mutex
{
public:
    using native_handle_type = myth_mutex_t*;
    
    mth_mutex() noexcept
    {
        // TODO: On old GCC (4.4), we cannot use {} on member initialization (?).
        myth_mutex_t m = MYTH_MUTEX_INITIALIZER;
        this->m_ = m;
    }
    
    mth_mutex(const mth_mutex&) = delete;
    mth_mutex& operator = (const mth_mutex&) = delete;
    
    ~mth_mutex() /*noexcept*/
    {
        myth_mutex_destroy(&this->m_);
        // Ignore the error.
    }
    
    bool try_lock()
    {
        const int ret = myth_mutex_trylock(&this->m_);
        
        if (ret == 0)
            return true;
        else if (ret == EBUSY)
            return false;
        else
            throw mth_error{};
    }
    
    void lock()
    {
        if (myth_mutex_lock(&this->m_) != 0)
            throw mth_error{};
    }
    
    
    void unlock()
    {
        if (myth_mutex_unlock(&this->m_) != 0)
            throw mth_error{};
    }
    
    native_handle_type native_handle() noexcept {
        return &this->m_;
    }
    
private:
    myth_mutex_t m_;
};

} // namespace cmpth

