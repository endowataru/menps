
#pragma once

#error "No longer maintained"

#include <mgbase/atomic.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/ult/mutex.hpp>
#include <mgbase/ult/condition_variable.hpp>

#define MGBASE_BLOCKING_BOUNDED_QUEUE_DISABLE_MUTEX

namespace mgbase {

template <
    typename Derived
,   typename Index
>
class mpsc_blocking_bounded_queue_counter_base
{
    typedef mpsc_blocking_bounded_queue_counter_base    self_type;
    typedef mgbase::ult::mutex                          mutex_type;
    typedef mgbase::ult::condition_variable             condition_variable_type;
    
public:
    struct enqueue_data
    {
        Index head;
        Index tail;
        bool  is_sleeping;
    };
    
    class enqueue_transaction
    {
    public:
        enqueue_transaction() MGBASE_NOEXCEPT
            : self_{MGBASE_NULLPTR} { }
        
        ~enqueue_transaction()
        {
            commit();
        }
    
    private:
        void commit()
        {
            if (self_ != MGBASE_NULLPTR) {
                if (is_sleeping()) {
                    self_->notify();
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
            data_ = other.data_;
            
            other.self_ = MGBASE_NULLPTR; // Important
            
            return *this;
        }
        
        MGBASE_EXPLICIT_OPERATOR bool() { return self_ != MGBASE_NULLPTR; }
        
        Index head() const MGBASE_NOEXCEPT { return data_.head; }
        Index tail() const MGBASE_NOEXCEPT { return data_.tail; }
        bool is_sleeping() const MGBASE_NOEXCEPT { return data_.is_sleeping; }
        
    private:
        friend class mpsc_blocking_bounded_queue_counter_base<Derived, Index>;
        
        enqueue_transaction(self_type& self, const enqueue_data& data) MGBASE_NOEXCEPT
            : self_{&self}
            , data_(data) // Use () because of the bug of GCC 4.4
            { }
        
        self_type*      self_;
        enqueue_data    data_;
    };
    
    enqueue_transaction try_enqueue(const Index diff)
    {
        const Index size = derived().size();
        MGBASE_ASSERT(diff < size);
        
        while (true)
        {
            // Load "con_" and "pro_" in a relaxed manner.
              const   Index con = con_.load(mgbase::memory_order_relaxed);
            /*const*/ Index pro = pro_.load(mgbase::memory_order_relaxed);
            
            // Evaluate the derived values.
            const Index head = con;
            const Index tail = pro >> 1;
            const bool is_sleeping = pro & 1;
            
            // Caluculate the next states.
            const Index next_tail = tail + diff;
            const Index next_pro = (next_tail << 1) | is_sleeping;
            
            const Index dist = next_tail - head;
            
            if (MGBASE_UNLIKELY(dist >= size))
            {
                MGBASE_LOG_DEBUG(
                    "msg:Queue is full.\t"
                    "con:{:x}\tpro:{:x}\tnext_pro:{:x}"
                ,   con
                ,   pro
                ,   next_pro
                );
                
                return {};
            }
            
            // Try to store the next tail.
            
            // It is dangerous when other producers return the same tail,
            // but in that case this CAS fails
            // because loading "pro_" must precede this CAS (constrained by memory_order_acquire)
            // and other producers must have already written the incremented value.
            if (MGBASE_LIKELY(
                pro_.compare_exchange_weak(pro, next_pro, mgbase::memory_order_acquire)
            ))
            {
                MGBASE_LOG_DEBUG(
                    "msg:Enqueued entry.\t"
                    "con:{:x}\tpro:{:x}\tnext_pro:{:x}"
                ,   con
                ,   pro
                ,   next_pro
                );
                
                // Return the range [tail, next_tail).
                return { *this, enqueue_data{ tail, next_tail, is_sleeping } };
            }
        }
    }
    
    Index number_of_enqueued() const MGBASE_NOEXCEPT
    {
        const Index head = con_.load(mgbase::memory_order_relaxed);
        const Index tail = pro_.load(mgbase::memory_order_relaxed) >> 1;
        return tail - head;
    }
    
    bool empty() const MGBASE_NOEXCEPT
    {
        return number_of_enqueued() == 0;
    }
    
    Index head() const MGBASE_NOEXCEPT
    {
        return con_.load(mgbase::memory_order_relaxed);
    }
    
    void wait()
    {
        #ifndef MGBASE_BLOCKING_BOUNDED_QUEUE_DISABLE_MUTEX
        mgbase::unique_lock<mutex_type> lc{ mtx_ };
        
        MGBASE_ASSERT((pro_.load(mgbase::memory_order_relaxed) & 1) == 0);
        pro_.fetch_add(1, mgbase::memory_order_acquire);
        
        while (number_of_enqueued() == 0) {
            cv_.wait(lc);
        }
        
        MGBASE_ASSERT((pro_.load(mgbase::memory_order_relaxed) & 1) == 1);
        pro_.fetch_sub(1, mgbase::memory_order_release);
        #endif
    }
    
    void notify()
    {
        #ifndef MGBASE_BLOCKING_BOUNDED_QUEUE_DISABLE_MUTEX
        mgbase::lock_guard<mutex_type> lc{ mtx_ };
        cv_.notify_one();
        #endif
    }
    
    void dequeue(const Index diff) MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(diff <= number_of_enqueued());
        
        // Allow producers to use the element at "head".
        // If a producer sees this modification,
        // then processing the element on the current thread must have completed.
        MGBASE_UNUSED
        const Index old_con = con_.fetch_add(diff, mgbase::memory_order_release);
        
        MGBASE_LOG_VERBOSE(
            "msg:Finished dequeuing.\t"
            "old_con:{:x}\tdiff:{}"
        ,   old_con
        ,   diff
        );
    }
    
protected:
    mpsc_blocking_bounded_queue_counter_base() MGBASE_NOEXCEPT
        : con_{0}
        , pro_{0} { }
    
private:
    mgbase::atomic<Index> con_; // head
    char pad1_[MGBASE_CACHE_LINE_SIZE - sizeof(Index)];
    mgbase::atomic<Index> pro_; // tail
    char pad2_[MGBASE_CACHE_LINE_SIZE - sizeof(Index)];
    mutex_type mtx_;
    condition_variable_type cv_;
    
    const Derived& derived() const MGBASE_NOEXCEPT { return static_cast<const Derived&>(*this); }
};

template <
    typename Index
,   Index Size
>
class static_mpsc_blocking_bounded_queue_counter
    : public mpsc_blocking_bounded_queue_counter_base<
        static_mpsc_blocking_bounded_queue_counter<Index, Size>
    ,   Index
    >
{
public:
    Index size() const MGBASE_NOEXCEPT { return Size; }
};

} // namespace mgbase

