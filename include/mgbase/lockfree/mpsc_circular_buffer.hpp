
#pragma once

#include <mgbase/lockfree/mpsc_circular_buffer_counter.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/assert.hpp>

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
        for (std::size_t i = 0; i < Size; ++i)
            buf_[i].visible = false;
    }
    
    ~mpsc_circular_buffer() MGBASE_EMPTY_DEFINITION
    
    bool try_push(const T& value)
    {
        mgbase::size_t tail;
        if (!counter_.try_enqueue(&tail))
            return false;
        
        element& elem = buf_[tail];
        
        MGBASE_ASSERT(!mgbase::atomic_load(&elem.visible));
        
        elem.value = value;
        
        mgbase::atomic_store_explicit(&elem.visible, true, mgbase::memory_order_release);
        
        return true;
    }
    
    T* peek() const
    {
        if (counter_.empty()) {
            MGBASE_LOG_VERBOSE(
                "msg:Peeked but queue is empty."
                /*"\thead:{:x}\ttail:{:x}"
            ,   head
            ,   tail*/
            );
            return MGBASE_NULLPTR;
        }
        
        element& elem = buf_[counter_.front()];
        const bool already_visible = 
            mgbase::atomic_load_explicit(&elem.visible, mgbase::memory_order_acquire);
        
        if (!already_visible) {
            MGBASE_LOG_VERBOSE("msg:Peeked but entry is not visible yet.");
            return MGBASE_NULLPTR;
        }
        
        return &elem.value;
    }
    
    void pop()
    {
        element& elem = buf_[counter_.front()];
        
        MGBASE_ASSERT(mgbase::atomic_load(&elem.visible));
        
        elem.visible = false; // Reset for the next use
        
        counter_.dequeue();
    }
    
private:
    mpsc_static_circular_buffer_counter<mgbase::size_t, Size> counter_;
    mgbase::scoped_ptr<element []> buf_;
};

} // namespace mgbase

