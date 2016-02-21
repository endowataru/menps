
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
    // TODO: consider cache line size
    struct /*MGBASE_ALIGNAS(MGBASE_CACHE_LINE_SIZE)*/ element
    {
        mgbase::atomic<bool> visible;
        T value;
    };
    
public:
    mpsc_circular_buffer()
    {
        buf_ = new element[Size];
        for (std::size_t i = 0; i < Size; ++i)
            buf_[i].visible.store(false, mgbase::memory_order_relaxed);
    }
    
    ~mpsc_circular_buffer() MGBASE_EMPTY_DEFINITION
    
    template <typename Func>
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_push_with_functor(Func func) {
        return try_multiple_push_with_functor(func, 1);
    }
    
    template <typename Func>
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_multiple_push_with_functor(Func func, const mgbase::size_t number_of_values)
    {
        mgbase::size_t tail;
        if (!counter_.try_enqueue(number_of_values, &tail))
            return false;
        
        for (mgbase::size_t i = 0; i < number_of_values; ++i)
        {
            const mgbase::size_t index = (tail + i) % counter_.size();
            element* const elem = &buf_[index];
            
            MGBASE_ASSERT(!elem->visible.load());
            
            func(&elem->value);
            
            if (i == (number_of_values - 1))
                elem->visible.store(true, mgbase::memory_order_release);
            else
                elem->visible.store(true, mgbase::memory_order_relaxed);
        }
        
        return true;
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_push(const T& value)
    {
        return try_push_multiple(&value, 1);
    }
    
    MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
    bool try_push_multiple(const T* values, mgbase::size_t number_of_values)
    {
        mgbase::size_t tail;
        if (!counter_.try_enqueue(number_of_values, &tail))
            return false;
        
        for (mgbase::size_t i = 0; i < number_of_values; ++i)
        {
            const mgbase::size_t index = (tail + i) % counter_.size();
            element* const elem = &buf_[index];
            
            MGBASE_ASSERT(!elem->visible.load());
            
            elem->value = values[i];
            
            if (i == (number_of_values - 1))
                elem->visible.store(true, mgbase::memory_order_release);
            else
                elem->visible.store(true, mgbase::memory_order_relaxed);
        }
        
        return true;
    }
    
    MGBASE_ALWAYS_INLINE
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
        const bool already_visible = elem.visible.load(mgbase::memory_order_acquire);
        
        if (!already_visible) {
            MGBASE_LOG_VERBOSE("msg:Peeked but entry is not visible yet.");
            return MGBASE_NULLPTR;
        }
        
        return &elem.value;
    }
    
    MGBASE_ALWAYS_INLINE
    void pop()
    {
        element& elem = buf_[counter_.front()];
        
        MGBASE_ASSERT(elem.visible.load());
        
        elem.visible.store(false, mgbase::memory_order_relaxed); // Reset for the next use
        
        counter_.dequeue();
    }
    
private:
    mpsc_static_circular_buffer_counter<mgbase::size_t, Size> counter_;
    mgbase::scoped_ptr<element []> buf_;
};

} // namespace mgbase

