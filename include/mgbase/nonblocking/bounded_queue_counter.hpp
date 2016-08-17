
#pragma once

#include <mgbase/atomic.hpp>
#include "circular_iterator.hpp"
#include <mgbase/iterator_facade.hpp>
#include <mgbase/utility/move.hpp>

#include <mgbase/logger.hpp>

namespace mgbase {

namespace detail {

/*
template <typename T>
struct bounded_queue_base_traits<???<T>>
{
    typedef ???             derived_type;
    typedef mgbase::size_t  index_type;
    typedef T               element_type;
    typedef entry<T>        entry_type;
};
*/

template <typename Traits>
class bounded_queue_entry_iterator
    : public circular_iterator<
        bounded_queue_entry_iterator<Traits>
    ,   typename Traits::entry_type
    >
{
    typedef Traits                              traits_type;
    typedef typename traits_type::derived_type  derived_type;
    typedef typename traits_type::index_type    index_type;
    typedef typename traits_type::entry_type    entry_type;
    
    typedef circular_iterator<bounded_queue_entry_iterator, entry_type>   base;
    
public:
    bounded_queue_entry_iterator(
        derived_type*       self
    ,   const index_type    index
    ) MGBASE_NOEXCEPT
        : base(index)
        , self_(self)
        { }
    
private:
    friend class circular_iterator<bounded_queue_entry_iterator, entry_type>;
    // friend base;
    
    entry_type& at(const index_type index) const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(self_ != MGBASE_NULLPTR);
        return self_->get_entry_at(index);
    }
    index_type size() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(self_ != MGBASE_NULLPTR);
        return self_->capacity();
    }
    
    derived_type* self_;
};

template <typename Traits>
class bounded_queue_iterator
    : public mgbase::iterator_facade<
        bounded_queue_iterator<Traits>
    ,   typename Traits::element_type
    ,   mgbase::random_access_iterator_tag
    >
{
    typedef Traits                              traits_type;
    typedef typename traits_type::index_type    index_type;
    typedef typename traits_type::element_type  element_type;
    
    typedef bounded_queue_entry_iterator<Traits>    entry_iterator_type;
    
public:
    explicit bounded_queue_iterator(const entry_iterator_type& itr) MGBASE_NOEXCEPT
        : itr_(itr) { }
    
private:
    friend class mgbase::iterator_core_access;
    
    element_type& dereference() const MGBASE_NOEXCEPT {
        return itr_->elem;
    }
    
    void increment() MGBASE_NOEXCEPT {
        ++itr_;
    }
    void decrement() MGBASE_NOEXCEPT {
        --itr_;
    }
    
    void advance(const mgbase::ptrdiff_t diff) {
        itr_ += diff;
    }
    
    bool equal(const bounded_queue_iterator& other) const MGBASE_NOEXCEPT {
        return itr_ == other.itr_;
    }
    mgbase::ptrdiff_t distance_to(const bounded_queue_iterator& other) const MGBASE_NOEXCEPT {
        return other.itr_ - itr_;
    }
    
    entry_iterator_type itr_;
};

template <typename Traits>
class bounded_queue_counter
{
    typedef Traits  traits_type;
    
protected:
    typedef typename traits_type::derived_type  derived_type;
    typedef typename traits_type::index_type    index_type;
    typedef typename traits_type::element_type  element_type;
    typedef typename traits_type::entry_type    entry_type;
    
private:
    typedef mgbase::atomic<index_type>   atomic_type;
    
protected:
    bounded_queue_counter() MGBASE_DEFAULT_NOEXCEPT
        : head_{0}
        , tail_{0}
        { }
    
    typedef bounded_queue_entry_iterator<traits_type>   entry_iterator;
    typedef bounded_queue_iterator<traits_type>         iterator;
    
private:
    struct transaction_data
    {
        index_type  from;
        index_type  to;
    };
    
    class transaction_base
    {
    public:
        // "Null transaction"
        transaction_base() MGBASE_NOEXCEPT
            : self_{MGBASE_NULLPTR}
            , data_{0, 0} // Zero-filled
            { }
        
        transaction_base(derived_type& self, const transaction_data& data) MGBASE_NOEXCEPT
            : self_{&self}
            , data_(data)
        {
            // Empty transaction must be invalid
            MGBASE_ASSERT(end() != begin());
        }
        
        transaction_base(const transaction_base&) = delete;
        
        transaction_base(transaction_base&& other) MGBASE_NOEXCEPT
            : self_{other.self_}
            , data_(other.data_)
        {
            other.self_ = MGBASE_NULLPTR;
        }
        
        ~transaction_base() {
            MGBASE_ASSERT(!valid());
        }
        
        transaction_base& operator = (const transaction_base&) = delete;
        
        transaction_base& operator = (transaction_base&& other) MGBASE_NOEXCEPT
        {
            MGBASE_ASSERT(!valid());
            
            self_ = other.self_;
            data_ = other.data_;
            
            other.self_ = MGBASE_NULLPTR;
            
            return *this;
        }
        
        bool valid() const MGBASE_NOEXCEPT {
            return self_ != MGBASE_NULLPTR;
        }
        
        iterator begin() const MGBASE_NOEXCEPT {
            return iterator(entry_begin());
        }
        iterator end() const MGBASE_NOEXCEPT {
            return iterator(entry_end());
        }
        
        entry_iterator entry_begin() const MGBASE_NOEXCEPT {
            return entry_iterator(self_, data_.from);
        }
        entry_iterator entry_end() const MGBASE_NOEXCEPT {
            return entry_iterator(self_, data_.to);
        }
        
        index_type size() const MGBASE_NOEXCEPT
        {
            return static_cast<index_type>(
                entry_end() - entry_begin()
            );
        }
        
    protected:
        void commit()
        {
            MGBASE_ASSERT(valid());
            self_ = MGBASE_NULLPTR;
        }
        
        derived_type*       self_;
        transaction_data    data_;
    };
    
public:
    void enqueue(const element_type& val)
    {
        this->enqueue(&val, &val + 1);
    }
    
    template <typename InputIterator>
    void enqueue(InputIterator first, const InputIterator last)
    {
        const auto num_req = mgbase::distance(first, last);
        MGBASE_ASSERT(num_req >= 0);
        
        while (true)
        {
            auto t = derived().try_enqueue(static_cast<index_type>(num_req));
            
            if (MGBASE_UNLIKELY(!t.valid())) {
                continue;
            }
            
            using mgbase::begin;
            using mgbase::end;
            
            auto t_itr = begin(t);
            const auto t_last = end(t);
            
            const auto num_enqueued = t_last - t_itr;
            MGBASE_ASSERT(num_enqueued <= num_req);
            
            if (num_enqueued == num_req)
            {
                for ( ; t_itr != t_last; ++t_itr, ++first)
                {
                    *t_itr = *first;
                    MGBASE_ASSERT(first != last);
                }
                
                MGBASE_ASSERT(first == last);
                
                t.commit();
                
                return;
            }
            else
                t.commit();
        }
    }
    
    element_type dequeue()
    {
        while (true)
        {
            element_type result;
            
            const index_type num_dequeued = this->dequeue(1, &result);
            
            if (num_dequeued == 1)
                return result;
        }
    }
    
    template <typename OutputIterator>
    MGBASE_WARN_UNUSED_RESULT
    index_type dequeue(const index_type max_num, OutputIterator result)
    {
        auto t = derived().try_dequeue(max_num);
        
        if (MGBASE_UNLIKELY(!t.valid())) {
            return 0;
        }
        
        using mgbase::begin;
        using mgbase::end;
        
        auto t_itr = begin(t);
        const auto t_last = end(t);
        
        MGBASE_ASSERT(static_cast<index_type>(t_last - t_itr) <= max_num);
        
        index_type num_dequeued = 0;
        
        for ( ; t_itr != t_last; ++t_itr, ++result, ++num_dequeued)
        {
            *result = *t_itr;
        }
        
        MGBASE_ASSERT(num_dequeued <= max_num);
        
        t.commit(num_dequeued);
        
        return num_dequeued;
    }
    
protected:
    typedef transaction_base    enqueue_transaction_base;
    typedef transaction_data    enqueue_transaction_data;
    
    typedef transaction_base    dequeue_transaction_base;
    typedef transaction_data    dequeue_transaction_data;
    
    entry_iterator make_entry_iterator_at(const index_type index)
    {
        return entry_iterator(&derived(), index);
    }
    iterator make_iterator_at(const index_type index)
    {
        return iterator(make_iterator_at(index));
    }
       
protected:
    atomic_type head_;
    
private:
    char pad1_[MGBASE_CACHE_LINE_SIZE - sizeof(index_type)];
    
protected:
    atomic_type tail_;
    
private:
    char pad2_[MGBASE_CACHE_LINE_SIZE - sizeof(index_type)];
    
private:
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
    const derived_type& derived() const MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace detail

} // namespace mgbase

