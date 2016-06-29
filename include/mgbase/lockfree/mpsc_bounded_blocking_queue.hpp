
#pragma once

#include <mgbase/lockfree/mpsc_circular_buffer_counter.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/ult/mutex.hpp>
#include <mgbase/ult/condition_variable.hpp>
#include <mgbase/ult/this_thread.hpp>

namespace mgbase {

template <
    typename T
,   mgbase::size_t Size
>
class mpsc_bounded_blocking_queue
    : mgbase::noncopyable
{
    typedef mgbase::ult::mutex                  mutex_type;
    typedef mgbase::ult::condition_variable     condition_variable_type;
    
    // TODO: consider cache line size
    struct /*MGBASE_ALIGNAS(MGBASE_CACHE_LINE_SIZE)*/ element
    {
        mgbase::atomic<bool> visible;
        T value;
    };
    
public:
    mpsc_bounded_blocking_queue()
        : wait_flag_{false}
    {
        buf_ = new element[Size];
        for (std::size_t i = 0; i < Size; ++i)
            buf_[i].visible.store(false, mgbase::memory_order_relaxed);
    }
    
    ~mpsc_bounded_blocking_queue() MGBASE_EMPTY_DEFINITION
    
private:
    struct default_push_closure {
        mpsc_bounded_blocking_queue& self;
        const T& value;
        
        void operator() (T* const dest) {
            *dest = value;
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT
    bool try_push(const T& value) {
        return try_push_with_functor(default_push_closure{*this, value});
    }
    
    template <typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_push_with_functor(Func&& func) {
        return try_multiple_push_with_functor(std::forward<Func>(func), 1);
    }
    
    template <typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_multiple_push_with_functor(Func func, const mgbase::size_t number_of_values)
    {
        mgbase::size_t tail;
        if (MGBASE_UNLIKELY(!counter_.try_enqueue(number_of_values, &tail))) {
            notify();
            return false;
        }
        
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
    
    void notify()
    {
        if (wait_flag_.load(mgbase::memory_order_acquire)) {
            mgbase::unique_lock<mutex_type> lc{ mtx_ };
            cv_.notify_one();
        }
    }
    
    #if 0
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
    #endif
private:
    struct default_pop_closure {
        mpsc_bounded_blocking_queue& self;
        T* result;
        
        template <typename Iterator>
        void operator() (Iterator first, Iterator last) {
            if (first != last) {
                *result = *first;
                ++first;
                MGBASE_ASSERT(first == last);
            }
        }
    };

public:
    T pop()
    {
        T result;
        pop_with_functor(default_pop_closure{*this, &result}, 1);
        return result;
    }
    
    template <typename Func>
    void pop_with_functor(Func func, const mgbase::size_t lim)
    {
        while (true)
        {
            while (MGBASE_UNLIKELY(counter_.empty())) // TODO: "unlikely" is correct?
            {
                mgbase::unique_lock<mutex_type> lc{ mtx_ };
                if (MGBASE_LIKELY(counter_.empty())) {
                    wait_flag_.store(true, mgbase::memory_order_release);
                    cv_.wait(lc);
                    wait_flag_.store(false, mgbase::memory_order_release);
                }
            }
            
            const mgbase::size_t num_enqueued = counter_.number_of_enqueued();
            
            const mgbase::size_t first = counter_.front();
            mgbase::size_t last = first;
            
            mgbase::size_t popped;
            for (popped = 0; popped < num_enqueued && popped < lim; ++popped)
            {
                if (MGBASE_UNLIKELY(
                    !buf_[last].visible.load(mgbase::memory_order_acquire)
                )) {
                    break;
                }
                
                last = (last + 1) % counter_.size();
            }
            
            if (popped == 0) {
                mgbase::ult::this_thread::yield();
                continue;
            }
            
            MGBASE_LOG_DEBUG(
                "msg:Popping elements.\t"
                "first:{:x}\tlast:{:x}\tpopped:{}"
            ,   first
            ,   last
            ,   popped
            );
            
            func(iterator{*this, first}, iterator{*this, last});
            
            for (mgbase::size_t i = 0; i < popped; ++i) {
                buf_[(first + i) % counter_.size()].visible.store(false, mgbase::memory_order_relaxed);
            }
            
            counter_.dequeue_multiple(popped);
            
            break;
        }
    }
    
    class iterator
    {
    public:
        iterator(mpsc_bounded_blocking_queue& self, mgbase::size_t index) MGBASE_NOEXCEPT
            : self_(self), index_(index) { }
        
        bool operator == (const iterator& other) MGBASE_NOEXCEPT {
            return index_ == other.index_;
        }
        bool operator != (const iterator& other) MGBASE_NOEXCEPT {
            return !(*this == other);
        }
        
        iterator& operator ++ () MGBASE_NOEXCEPT {
            index_ = (index_ + 1) % self_.counter_.size();
            return *this;
        }
        
        T& operator* () const MGBASE_NOEXCEPT {
            MGBASE_ASSERT(index_ < self_.counter_.size());
            return self_.buf_[index_].value;
        }
        
    private:
        mpsc_bounded_blocking_queue& self_;
        mgbase::size_t index_;
    };
    
    
    #if 0
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
    #endif
    
private:
    mpsc_static_circular_buffer_counter<mgbase::size_t, Size> counter_;
    mutex_type mtx_;
    condition_variable_type cv_;
    mgbase::atomic<bool> wait_flag_;
    mgbase::scoped_ptr<element []> buf_;
};

} // namespace mgbase

