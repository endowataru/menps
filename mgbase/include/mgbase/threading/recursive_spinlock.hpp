
#pragma once

#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/this_thread.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/threading/this_thread.hpp>

namespace mgbase {

class recursive_spinlock
{
public:
    recursive_spinlock() MGBASE_NOEXCEPT
        : count_(0)
    {
        
    }
    
    bool try_lock()
    {
        const auto this_id = mgbase::this_thread::get_id();
        
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        if (id_ == this_id) {
            // Already owns lock.
            ++count_;
            return true;
        }
        else if (count_ == 0)
        {
            // No thread is locking.
            
            id_ = mgbase::this_thread::get_id();
            
            MGBASE_ASSERT(count_ == 0);
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
            mgbase::this_thread::yield();
        }
    }
    
    void unlock()
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        MGBASE_ASSERT(count_ > 0);
        MGBASE_ASSERT(id_ == mgbase::this_thread::get_id());
        
        --count_;
    }
    
private:
    mgbase::spinlock    lock_;
    thread_id           id_;
    mgbase::size_t      count_;
};

} // namespace mgbase

