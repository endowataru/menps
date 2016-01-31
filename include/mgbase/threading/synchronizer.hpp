
#pragma once

#include <mgbase/mutex.hpp>
#include <mgbase/condition_variable.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

class synchronizer
    : mgbase::noncopyable
{
public:
    synchronizer()
        : request_flag_(false), reply_flag_(false) { }
    
    void start(mgbase::unique_lock<mgbase::mutex>& lc)
    {
        MGBASE_ASSERT(lc.owns_lock());
        
        MGBASE_ASSERT(!request_flag_);  // false
        MGBASE_ASSERT(!reply_flag_);    // false
        
        request_flag_ = true;
        
        while (!reply_flag_) { // -> true
            // Note: reply thread might not be waiting now
            reply_cv_.notify_one();
            
            request_cv_.wait(lc);
        }
        
        MGBASE_ASSERT(request_flag_);   // true
        MGBASE_ASSERT(reply_flag_);     // true
    }
    
    void finish(mgbase::unique_lock<mgbase::mutex>& lc)
    {
        MGBASE_ASSERT(lc.owns_lock());
        
        MGBASE_ASSERT(request_flag_);   // true
        MGBASE_ASSERT(reply_flag_);     // true
        
        request_flag_ = false;
        reply_cv_.notify_one();
        
        while (reply_flag_) { // -> false
            request_cv_.wait(lc);
        }
        
        MGBASE_ASSERT(!request_flag_);  // false
        MGBASE_ASSERT(!reply_flag_);    // false
    }
    
    void wait(mgbase::unique_lock<mgbase::mutex>& lc)
    {
        MGBASE_ASSERT(lc.owns_lock());
        
        MGBASE_ASSERT(request_flag_);   // true
        MGBASE_ASSERT(!reply_flag_);    // false
        
        reply_flag_ = true;
        request_cv_.notify_one();
        
        while (request_flag_) { // -> false
            reply_cv_.wait(lc);
        }
        
        MGBASE_ASSERT(!request_flag_);  // false
        MGBASE_ASSERT(reply_flag_);     // true
        
        reply_flag_ = false;
        request_cv_.notify_one();
    }

private:
    mgbase::condition_variable  request_cv_;
    mgbase::condition_variable  reply_cv_;
    bool                        request_flag_;
    bool                        reply_flag_;
};

} // namespace mgbase

