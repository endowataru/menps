
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_qdlock_cv
{
    using ult_itf_type = typename P::ult_itf_type;
    using uncond_variable_type = typename ult_itf_type::uncond_variable;
    
    using qdlock_mutex_type = typename P::qdlock_mutex_type;
    using qdlock_unique_lock_type = typename P::qdlock_unique_lock_type;
    
    struct wait_entry {
        uncond_variable_type    uv;
        wait_entry*             next;
    };
    
public:
    void wait(qdlock_unique_lock_type& lk)
    {
        wait_entry e;
        
        e.next = this->waiting_;
        this->waiting_ = &e;
        
        const auto mtx = lk.release();
        
        e.uv.wait_with(
            [mtx] () {
                mtx->unlock();
                return true;
            }
        );
        
        // Lock again here.
        lk = qdlock_unique_lock_type(*mtx);
    }
    
    template <typename Pred>
    void wait(qdlock_unique_lock_type& lk, Pred pred)
    {
        while (!pred()) {
            this->wait(lk);
        }
    }
    
    void notify_one()
    {
        if (const auto we = this->waiting_) {
            we->uv.notify();
            this->waiting_ = we->next;
        }
    }
    
    void notify_all()
    {
        auto we = this->waiting_;
        while (we != nullptr) {
            we->uv.notify();
            we = we->next;
        }
        this->waiting_ = nullptr;
    }
    
private:
    wait_entry* waiting_ = nullptr;
};

} // namespace meult
} // namespace menps

