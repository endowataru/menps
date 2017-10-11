
#pragma once

#include "interprocess_mutex.hpp"
#include "interprocess_condition.hpp"

namespace mgbase {

template <typename T, mgbase::size_t Size>
class static_message_queue
{
public:
    static_message_queue()
        : head_{0}
        , tail_{0} { }
    
    void send(const T& val)
    {
        mgbase::unique_lock<mgbase::interprocess_mutex> lc(mtx_);
        
        while (size_ >= Size) {
            cond_.wait(lc);
        }
        
        arr_[tail_] = val;
        
        increment(&tail_);
        ++size_;
        
        cond_.notify_one();
    }
    
    void receive(T* const dest)
    {
        mgbase::unique_lock<mgbase::interprocess_mutex> lc(mtx_);
        
        while (size_ == 0) {
            cond_.wait(lc);
        }
        
        *dest = arr_[head_];
        
        increment(&head_);
        --size_;
        
        cond_.notify_one();
    }
    
private:
    static void increment(mgbase::size_t* const index) {
        *index = (*index + 1) % Size;
    }
    
    interprocess_mutex mtx_;
    interprocess_condition cond_;
    mgbase::size_t head_, tail_, size_;
    T arr_[Size];
};

} // namespace mgbase

