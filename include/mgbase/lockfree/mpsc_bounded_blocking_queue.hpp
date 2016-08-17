
#pragma once

#error "No longer maintained"

#include <mgbase/lockfree/mpsc_bounded_blocking_queue_counter.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

template <
    typename T
,   mgbase::size_t Size
>
class mpsc_bounded_blocking_queue
    : mgbase::noncopyable
{
    // TODO: consider cache line size
    struct /*MGBASE_ALIGNAS(MGBASE_CACHE_LINE_SIZE)*/ element
    {
        mgbase::atomic<bool> visible;
        T value;
    };
    
public:
    mpsc_bounded_blocking_queue()
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
        counter_.notify();
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
    
private:
    class pop_result
    {
    public:
        pop_result(
            mpsc_bounded_blocking_queue&    self
        ,   const mgbase::size_t            first
        ,   const mgbase::size_t            last
        ,   const mgbase::size_t            popped
        )
            : self_(self)
            , first_{first}
            , last_{last}
            , popped_{popped}
        {
        }
        
        ~pop_result()
        {
            auto& counter = self_.counter_;
            const auto size = counter.size();
            
            for (mgbase::size_t i = 0; i < popped_; ++i) {
                self_.buf_[(first_ + i) % size].visible.store(false, mgbase::memory_order_relaxed);
            }
            
            counter.dequeue_multiple(popped_);
        }
        
        pop_result(const pop_result&) = delete;
        pop_result& operator = (const pop_result&) = delete;
        
        #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
        pop_result(pop_result&&) = default;
        #else
        pop_result(pop_result&& other)
            : self_(other.self_), first_{other.first_}, last_{other.last_}, popped_{other.popped_} { }
        #endif
        
        iterator begin() { return {self_, first_}; }
        iterator end() { return {self_, last_}; }
        
    private:
        mpsc_bounded_blocking_queue& self_;
        mgbase::size_t first_;
        mgbase::size_t last_;
        mgbase::size_t popped_;
    };
    
public:
    pop_result pop(const mgbase::size_t lim)
    {
        if (counter_.empty())
            counter_.wait();
        
        const mgbase::size_t num_enqueued = counter_.number_of_enqueued();
        
        const mgbase::size_t first = counter_.front();
        mgbase::size_t last = first;
        
        mgbase::size_t popped;
        for (popped = 0; popped < num_enqueued && popped < lim; ++popped)
        {
            if (MGBASE_UNLIKELY(
                !buf_[last].visible.load(mgbase::memory_order_acquire)
            )) {
                if (popped == 0) {
                    // If there's no available element but some are being enqueued
                    mgbase::ult::this_thread::yield();
                }
                break;
            }
            
            last = (last + 1) % counter_.size();
        }
        
        MGBASE_LOG_DEBUG(
            "msg:Popping elements.\t"
            "first:{:x}\tlast:{:x}\tpopped:{}"
        ,   first
        ,   last
        ,   popped
        );
        
        return { *this, first, last, popped };
    }
    
    T pop()
    {
        while (true)
        {
            auto ret = pop(1);
            
            auto first = ret.begin();
            auto last = ret.end();
            
            if (MGBASE_LIKELY(first != last))
            {
                const auto r = *first;
                
                MGBASE_ASSERT(++first == last);
                
                return r;
            }
        }
    }
    
private:
    mpsc_bounded_blocking_queue_counter<mgbase::size_t, Size> counter_;
    mgbase::scoped_ptr<element []> buf_;
};

} // namespace mgbase

