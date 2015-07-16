
#pragma once

#include <pthread.h>

namespace mgbase {

struct thread_error { };

class thread
    : noncopyable
{
public:
    explicit thread(void (*func))
    {
        if (pthread_create(&th_, MBBASE_NULLPTR, func, MBBASE_NULLPTR) != 0)
            throw thread_error();
    }
    
    void join()
    {
        pthread_join(thread, MBBASE_NULLPTR);
    }

private:
    pthread_t th_;
};


}

