
#pragma once

#include <mgbase/lockfree/mpsc_circular_buffer_counter.hpp>
#include <mgbase/scoped_ptr.hpp>

namespace mgbase {

// Reference:
// http://psy-lob-saw.blogspot.jp/2014/04/notes-on-concurrent-ring-buffer-queue.html

template <
    typename T
,   mgbase::size_t Size
>
class mpsc_circular_buffer
    : mgbase::noncopyable
{
    struct MGBASE_ALIGNAS(MGBASE_CACHE_LINE_SIZE) element
    {
        bool visible;
        T value;
    };
    
public:
    mpsc_circular_buffer()
    {
        buf_ = new element[Size];
    }
    
    ~mpsc_circular_buffer() MGBASE_EMPTY_DEFINITION
    
    bool try_push(const T& value)
    {
        mgbase::size_t tail;
        if (!counter_.try_enqueue(&tail))
            return false;
        
        element& elem = buf_[tail];
        elem.value = value;
        
        mgbase::atomic_store_explicit(&elem.visible, true, mgbase::memory_order_release);
        
        return true;
    }
    
    bool try_pop(T* dest)
    {
        mgbase::size_t head;
        if (!counter_.try_start_dequeue(&head))
            return false;
        
        element& elem = buf_[head];
        const bool already_visible =
            mgbase::atomic_load_explicit(&elem.visible, mgbase::memory_order_acquire);
        
        if (!already_visible)
            return false;
        
        *dest = elem.value;
        
        counter_.finish_dequeue(head);
        
        return true;
    }
    
private:
    mpsc_static_circular_buffer_counter<mgbase::size_t, Size> counter_;
    mgbase::scoped_ptr<element []> buf_;
};

} // namespace mgbase

