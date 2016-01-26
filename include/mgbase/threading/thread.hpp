
#pragma once

#include <mgbase/threading/thread_id.hpp>

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
    
public:
    thread() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename Func>
    explicit thread(const Func& func)
    {
        run(func);
    }
    
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
    
    void join() {
        pthread_join(th_, MGBASE_NULLPTR);
    }

private:
    pthread_t th_;
};

} // namespace mgbase

