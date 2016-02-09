
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
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        
        if (is_offloading_) {
            while (!try_sync(lc)) { }
        }
        else {
            MGBASE_LOG_DEBUG("msg:Directly locked the communication lock.");
            lc.release();
        }
    }
    
    bool try_lock()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_, mgbase::try_to_lock);
        
        if (!lc.owns_lock())
            return false;
        
        if (is_offloading_)
            return try_sync(lc);
        else {
            MGBASE_LOG_DEBUG("msg:Directly tried and locked the communication lock.");
            lc.release();
            return true;
        }
    }
    
    void unlock()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_, mgbase::adopt_lock);
        
        if (is_offloading_)
            sync_.finish(lc);
        else {
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
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        
        MGBASE_ASSERT(!is_offloading_);
        is_offloading_ = true;
    }
    
    void stop_offloading()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        
        MGBASE_ASSERT(is_offloading_);
        is_offloading_ = false;
    }
    
    void wait()
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx_);
        sync_.wait(lc);
    }
    
protected:
    virtual bool try_enqueue_unlock() = 0;
    
private:
    mgbase::mutex           mtx_;
    mgbase::synchronizer    sync_;
    bool                    is_offloading_;
};

} // namespace mgcom

