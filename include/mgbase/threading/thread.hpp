
#pragma once

#include <mgbase/lang.hpp>
#include <pthread.h>

namespace mgbase {

struct thread_error { };

class thread
    : noncopyable
{
public:
    /*explicit thread(void (*func))
    {
        
    }*/
    thread() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    /*template <typename T>
    void run(void (T::*func)(), T* self) {
        typedef void (T::*func_ptr)();
        
        struct starter {
            static void* start(void* ptr) {
                thread* th = static_cast<thread*>(ptr);
                T* self = static_cast<T*>(th->self_);
                func_ptr func = reinterpret_cast<func_ptr>(th->func_);
                
                self->*func();
                
                return MGBASE_NULLPTR;
            }
        };
        
        self_ = self;
        func_ = reinterpret_cast<uint64_t>(func);
        
        if (pthread_create(&th_, MGBASE_NULLPTR, &starter::start, this) != 0)
            throw thread_error();
    }*/
    template <typename T>
    void run(T);
    
    void join() {
        pthread_join(th_, MGBASE_NULLPTR);
    }

private:
    pthread_t th_;
};


}

