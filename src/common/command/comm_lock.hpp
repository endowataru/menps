
#pragma once

#include <mgbase/mutex.hpp>
#include <mgbase/threading/synchronizer.hpp>

namespace mgcom {

class comm_lock
    : mgbase::noncopyable
{
protected:
    comm_lock() : is_offloading_(false) { }
    
public:
    virtual ~comm_lock() MGBASE_EMPTY_DEFINITION
    
public:
    void lock()
    {
        // Lock the mutex for offloading first.
        mgbase::unique_lock<mgbase::mutex> lc(offload_mtx_);
        
        if (is_offloading_) {
            // Queue the lock.
            while (!try_sync(lc)) { }
        }
        else {
            MGBASE_LOG_DEBUG("msg:Directly locked the communication lock.");
            
            // Lock the mutex for communication.
            comm_mtx_.lock();
        }
    }
    
    bool try_lock()
    {
        // Try to lock the mutex for offloading first.
        mgbase::unique_lock<mgbase::mutex> lc(offload_mtx_, mgbase::try_to_lock);
        
        if (!lc.owns_lock())
            return false;
        
        if (is_offloading_) {
            // Try to queue the lock.
            return try_sync(lc);
        }
        else {
            MGBASE_LOG_DEBUG("msg:Directly tried and locked the communication lock.");
            
            // Try to lock the mutex for communication.
            return comm_mtx_.try_lock();
        }
    }
    
    void unlock()
    {
        mgbase::unique_lock<mgbase::mutex> lc(offload_mtx_);
        
        if (is_offloading_) {
            // Finish the temporary critical section.
            sync_.finish(lc);
        }
        else {
            // This thread is already locking the mutex.
            comm_mtx_.unlock();
            
            MGBASE_LOG_DEBUG("msg:Directly unlocked the communication lock.");
        }
    }
    
private:
    bool try_sync(mgbase::unique_lock<mgbase::mutex>& lc)
    {
        MGBASE_ASSERT(lc.owns_lock());
        
        if (try_enqueue_unlock()) {
            sync_.start(lc);
            return true;
        }
        else
            return false;
    }
    
public:
    void start_offloading()
    {
        {
            mgbase::unique_lock<mgbase::mutex> lc(offload_mtx_);
            
            MGBASE_ASSERT(!is_offloading_);
            
            // Change the state to offloading.
            is_offloading_ = true;
        }
        
        // Lock the communication lock.
        comm_mtx_.lock();
    }
    
    void wait()
    {
        mgbase::unique_lock<mgbase::mutex> lc(offload_mtx_);
        sync_.wait(lc);
    }
    
protected:
    virtual bool try_enqueue_unlock() = 0;
    
private:
    mgbase::mutex           comm_mtx_;
    mgbase::mutex           offload_mtx_;
    mgbase::synchronizer    sync_;
    bool                    is_offloading_;
};

} // namespace mgcom

