
#pragma once

#include <mgbase/lockfree/mpsc_blocking_bounded_queue_counter.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/assert.hpp>

namespace mgbase {

template <
    typename        T
,   mgbase::size_t  Size
>
class mpsc_blocking_bounded_queue
    : mgbase::noncopyable
{
    typedef mpsc_blocking_bounded_queue     self_type;
    typedef static_mpsc_blocking_bounded_queue_counter<mgbase::size_t, Size>
        counter_type;
    
    // TODO: consider cache line size
    struct /*MGBASE_ALIGNAS(MGBASE_CACHE_LINE_SIZE)*/ element
    {
        mgbase::atomic<bool> visible;
        T value;
    };
    
public:
    mpsc_blocking_bounded_queue()
    {
        buf_ = new element[Size];
        for (mgbase::size_t i = 0; i < Size; ++i)
            buf_[i].visible.store(false, mgbase::memory_order_relaxed);
    }
    
    ~mpsc_blocking_bounded_queue() MGBASE_EMPTY_DEFINITION
    
    class iterator
    {
    public:
        iterator(const iterator&) /*MGBASE_NOEXCEPT*/ = default;
        iterator& operator = (const iterator&) /*MGBASE_NOEXCEPT*/ = default;
        
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
        
        // TODO: Remote this
        element& get_element() {
            return self_.buf_[index_];
        }
        
    private:
        friend class mpsc_blocking_bounded_queue<T, Size>;
        
        iterator(self_type& self, const mgbase::size_t index) MGBASE_NOEXCEPT
            : self_(self), index_{index} { }
        
        self_type& self_;
        mgbase::size_t index_;
    };
    
    class enqueue_transaction
    {
        typedef typename counter_type::enqueue_transaction  basic_type;
        
    public:
        ~enqueue_transaction()
        {
            commit();
        }
        
    private:
        void commit()
        {
            MGBASE_ASSERT((self_ && basic_) || (!self_ && !basic_));
            
            if (self_ != MGBASE_NULLPTR)
            {
                const auto last = end();
                
                for (auto itr = begin(); itr != last; ++itr)
                {
                    auto& elem = itr.get_element();
                    
                    MGBASE_ASSERT(!elem.visible.load());
                    
                    // TODO: reduce memory barriers
                    elem.visible.store(true, mgbase::memory_order_release);
                }
            }
            
            self_ = MGBASE_NULLPTR;
        }
    
    public:
        enqueue_transaction(const enqueue_transaction&) = delete;
        enqueue_transaction& operator = (const enqueue_transaction&) = delete;
        
        enqueue_transaction(enqueue_transaction&& other) MGBASE_NOEXCEPT 
            : self_{MGBASE_NULLPTR}
        {
            *this = std::move(other);
        }
        enqueue_transaction& operator = (enqueue_transaction&& other) MGBASE_NOEXCEPT {
            // Destroy *this first.
            commit();
            
            self_ = other.self_;
            basic_ = std::move(other.basic_);
            
            other.self_ = MGBASE_NULLPTR; // Important
            
            return *this;
        }
        
        MGBASE_EXPLICIT_OPERATOR bool() { return self_ != MGBASE_NULLPTR; }
        
        iterator begin() { return {*self_, basic_.head() % size() }; }
        iterator end() { return {*self_, basic_.tail() % size() }; }
        
    private:
        mgbase::size_t size() MGBASE_NOEXCEPT {
            MGBASE_ASSERT(self_ != MGBASE_NULLPTR);
            return self_->counter_.size();
        }
        
        friend class mpsc_blocking_bounded_queue<T, Size>;
        
        enqueue_transaction() MGBASE_NOEXCEPT
            : self_{MGBASE_NULLPTR}, basic_{} { }
        
        enqueue_transaction(self_type& self, basic_type&& basic) MGBASE_NOEXCEPT
            : self_{&self}, basic_{std::move(basic)} { }
        
        self_type*  self_;
        basic_type  basic_;
    };
    
    enqueue_transaction try_enqueue(const mgbase::size_t number_of_values)
    {
        auto t = counter_.try_enqueue(number_of_values);
        
        if (t)
            return { *this, std::move(t) };
        else
            return {};
    }
    
private:
    struct default_push_closure {
        self_type& self;
        const T& value;
        
        void operator() (T* const dest) {
            *dest = value;
        }
    };
    
public:
    MGBASE_WARN_UNUSED_RESULT MGBASE_DEPRECATED
    bool try_push(const T& value)
    {
        return try_push_with_functor(default_push_closure{*this, value});
    }
    
    template <typename Func>
    MGBASE_WARN_UNUSED_RESULT MGBASE_DEPRECATED
    bool try_push_with_functor(Func&& func)
    {
        auto t = try_enqueue(1);
        
        if (t) {
            func(&*t.begin());
            return true;
        }
        else
            return false;
    }
    
    #if 0
    
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
        const bool was_empty = counter_.empty();
        
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
        
        if (was_empty) {
            counter_.notify();
        }
        
        return true;
    }
    #endif
    
public:
    
    class dequeue_transation
    {
    public:
        dequeue_transation(
            self_type&    self
        ,   const mgbase::size_t            first
        ,   const mgbase::size_t            last
        ,   const mgbase::size_t            popped
        )
            : self_(&self)
            , first_{first}
            , last_{last}
            , popped_{popped}
        {
        }
        
        ~dequeue_transation()
        {
            commit();
        }
        
    private:
        void commit()
        {
            if (self_ != MGBASE_NULLPTR)
            {
                auto& counter = self_->counter_;
                const auto size = counter.size();
                
                for (mgbase::size_t i = 0; i < popped_; ++i)
                {
                    auto& elem = self_->buf_[(first_ + i) % size];
                    
                    MGBASE_ASSERT(elem.visible.load());
                    
                    elem.visible.store(false, mgbase::memory_order_relaxed);
                }
                
                counter.dequeue(popped_);
            }
            
            self_ = MGBASE_NULLPTR;
        }
        
    public:
        dequeue_transation(const dequeue_transation&) = delete;
        dequeue_transation& operator = (const dequeue_transation&) = delete;
        
        dequeue_transation(dequeue_transation&& other)
            : self_{MGBASE_NULLPTR}
        {
            *this = std::move(other);
        }
        
        dequeue_transation& operator = (dequeue_transation&& other) {
            // Destory *this first.
            commit();
            
            self_ = other.self_;
            first_ = other.first_;
            last_ = other.last_;
            popped_ = other.popped_;
            
            other.self_ = MGBASE_NULLPTR; // Important
            
            return *this;
        }
        
        iterator begin() { return {*self_, first_}; }
        iterator end() { return {*self_, last_}; }
        
    private:
        self_type* self_;
        mgbase::size_t first_;
        mgbase::size_t last_;
        mgbase::size_t popped_;
    };
    
public:
    dequeue_transation dequeue(const mgbase::size_t lim)
    {
        if (counter_.empty())
            counter_.wait();
        
        const mgbase::size_t num_enqueued = counter_.number_of_enqueued();
        
        const mgbase::size_t first = counter_.head() % counter_.size();
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
        
        MGBASE_LOG_DEBUG(
            "msg:Popping elements.\t"
            "first:{:x}\tlast:{:x}\tpopped:{}"
        ,   first
        ,   last
        ,   popped
        );
        
        return { *this, first, last, popped };
    }
    
    T dequeue()
    {
        while (true)
        {
            auto t = dequeue(1);
            
            auto first = t.begin();
            const auto last = t.end();
            
            if (MGBASE_LIKELY(first != last))
            {
                const auto r = *first;
                
                MGBASE_ASSERT(++first == last);
                
                return r;
            }
        }
    }
    
private:
    counter_type counter_;
    mgbase::scoped_ptr<element []> buf_;
};

} // namespace mgbase


