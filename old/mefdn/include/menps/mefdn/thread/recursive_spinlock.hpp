
#pragma once

#include <menps/mefdn/thread/spinlock.hpp>
#include <menps/mefdn/thread/this_thread.hpp>
#include <menps/mefdn/thread/lock_guard.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace mefdn {

class recursive_spinlock
{
public:
    recursive_spinlock() noexcept
        : count_(0)
    {
        
    }
    
    bool try_lock()
    {
        const auto this_id = mefdn::this_thread::get_id();
        
        mefdn::lock_guard<mefdn::spinlock> lc(lock_);
        
        if (id_ == this_id) {
            // Already owns lock.
            ++count_;
            return true;
        }
        else if (count_ == 0)
        {
            // No thread is locking.
            
            id_ = mefdn::this_thread::get_id();
            
            MEFDN_ASSERT(count_ == 0);
            count_ = 1;
            
            return true;
        }
        else
            return false;
    }
    
    void lock()
    {
        while (!try_lock())
        {
            mefdn::this_thread::yield();
        }
    }
    
    void unlock()
    {
        mefdn::lock_guard<mefdn::spinlock> lc(lock_);
        
        MEFDN_ASSERT(count_ > 0);
        MEFDN_ASSERT(id_ == mefdn::this_thread::get_id());
        
        --count_;
    }
    
private:
    mefdn::spinlock     lock_;
    thread::id          id_;
    mefdn::size_t       count_;
};

} // namespace mefdn
} // namespace menps

