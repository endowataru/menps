
#pragma once

#include <mgbase/threading/thread_id.hpp>

#ifdef MGBASE_CPP11_SUPPORTED

#include <thread>

namespace mgbase {

using std::thread;

} // namespace mgbase

#else

#include <mgbase/move.hpp>

namespace mgbase {

struct thread_error { };

class thread
{
    template <typename Func>
    struct starter
    {
        static void* pass(void* ptr)
        {
            Func* const func_ptr = static_cast<Func*>(ptr);
            
            (*func_ptr)();
            
            delete func_ptr;
            
            return MGBASE_NULLPTR;
        }
    };
    
    MGBASE_MOVABLE_BUT_NOT_COPYABLE(thread)
    
public:
    thread() MGBASE_NOEXCEPT
        : running_(false) { }
    
    thread(MGBASE_RV_REF(thread) other) MGBASE_NOEXCEPT
    {
        if (other.running_) {
            this->running_ = true;
            this->th_ = other.th_;
            other.running_ = false;
        }
    }
    
    template <typename Func>
    explicit thread(const Func& func)
        : running_(true)
    {
        run(func);
    }
    
    thread& operator = (MGBASE_RV_REF(thread) other) MGBASE_NOEXCEPT {
        if (other.running_) {
            this->running_ = true;
            this->th_ = other.th_;
            other.running_ = false;
        }
        return *this;
    }
    
    void join() {
        pthread_join(th_, MGBASE_NULLPTR);
        running_ = false;
    }
    

private:
    template <typename Func>
    void run(const Func& func)
    {
        Func* const func_ptr = new Func(func);
        
        const int ret = pthread_create(
            &th_
        ,   MGBASE_NULLPTR
        ,   &starter<Func>::pass
        ,   func_ptr
        );
        if (ret != 0)
            throw thread_error();
    }
    
    bool running_;
    pthread_t th_;
};

} // namespace mgbase

#endif

