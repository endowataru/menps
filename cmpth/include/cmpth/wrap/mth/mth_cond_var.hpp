
#pragma once

#include <cmpth/wrap/mth/mth.hpp>

namespace cmpth {

class mth_cond_var
{
    using mutex_type = mth_mutex;
    
public:
    mth_cond_var()
    {
        // TODO: On old GCC (4.4), we cannot use {} on member initialization (?).
        myth_cond_t cond = MYTH_COND_INITIALIZER;
        this->cond_ = cond;
    }
    
    mth_cond_var(const mth_cond_var&) = delete;
    mth_cond_var& operator = (const mth_cond_var&) = delete;
    
    ~mth_cond_var()
    {
        if (myth_cond_destroy(&this->cond_) != 0) {
            // Ignore because this is in a destructor
        }
    }
    
    void wait(fdn::unique_lock<mutex_type>& lk)
    {
        if (!lk.owns_lock())
            throw mth_error{};
        
        const auto mtx = lk.mutex();
        if (mtx == nullptr)
            throw mth_error{};
        
        const auto mtx_ptr = mtx->native_handle();
        
        const auto ret = myth_cond_wait(&this->cond_, mtx_ptr);
        if (ret != 0)
            throw mth_error{};
    }
    
    template <typename Predicate>
    void wait(fdn::unique_lock<mutex_type>& lc, Predicate pred)
    {
        while (!pred())
        {
            wait(lc);
        }
    }
    
    void notify_one()
    {
        if (myth_cond_signal(&this->cond_) != 0)
            throw mth_error{};
    }
    
    void notify_all()
    {
        if (myth_cond_broadcast(&this->cond_) != 0)
            throw mth_error{};
    }
    
private:
    myth_cond_t cond_;
};

} // namespace cmpth

