
#pragma once

#include "circular_iterator.hpp"
#include <menps/mefdn/iterator_facade.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace mefdn {

namespace detail {

/*
template <typename T>
struct bounded_queue_base_traits<???<T>>
{
    typedef ???             derived_type;
    typedef mefdn::size_t  index_type;
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
    ) noexcept
        : base(index)
        , self_(self)
    { }
    
private:
    friend class circular_iterator<bounded_queue_entry_iterator, entry_type>;
    // friend base;
    
    entry_type& at(const index_type index) const noexcept {
        MEFDN_ASSERT(self_ != nullptr);
        return self_->get_entry_at(index);
    }
    index_type size() const noexcept {
        MEFDN_ASSERT(self_ != nullptr);
        return self_->capacity();
    }
    
    derived_type* self_;
};

template <typename Traits>
class bounded_queue_iterator
    : public mefdn::iterator_facade<
        bounded_queue_iterator<Traits>
    ,   typename Traits::element_type
    ,   mefdn::random_access_iterator_tag
    >
{
    typedef Traits                              traits_type;
    typedef typename traits_type::index_type    index_type;
    typedef typename traits_type::element_type  element_type;
    
    typedef bounded_queue_entry_iterator<Traits>    entry_iterator_type;
    
public:
    explicit bounded_queue_iterator(const entry_iterator_type& itr) noexcept
        : itr_(itr) { }
    
private:
    friend class mefdn::iterator_core_access;
    
    element_type& dereference() const noexcept {
        return itr_->elem;
    }
    
    void increment() noexcept {
        ++itr_;
    }
    void decrement() noexcept {
        --itr_;
    }
    
    void advance(const mefdn::ptrdiff_t diff) {
        itr_ += diff;
    }
    
    bool equal(const bounded_queue_iterator& other) const noexcept {
        return itr_ == other.itr_;
    }
    mefdn::ptrdiff_t distance_to(const bounded_queue_iterator& other) const noexcept {
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
    typedef mefdn::atomic<index_type>   atomic_type;
    
protected:
    bounded_queue_counter() noexcept
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
        transaction_base() noexcept
            : self_{nullptr}
            , data_{0, 0} // Zero-filled
            { }
        
        transaction_base(derived_type& self, const transaction_data& data) noexcept
            : self_{&self}
            , data_(data)
        {
            // Empty transaction must be invalid
            MEFDN_ASSERT(end() != begin());
        }
        
        transaction_base(const transaction_base&) = delete;
        
        transaction_base(transaction_base&& other) noexcept
            : self_{other.self_}
            , data_(other.data_)
        {
            other.self_ = nullptr;
        }
        
        ~transaction_base() {
            MEFDN_ASSERT(!valid());
        }
        
        transaction_base& operator = (const transaction_base&) = delete;
        
        transaction_base& operator = (transaction_base&& other) noexcept
        {
            MEFDN_ASSERT(!valid());
            
            self_ = other.self_;
            data_ = other.data_;
            
            other.self_ = nullptr;
            
            return *this;
        }
        
        bool valid() const noexcept {
            return self_ != nullptr;
        }
        
        iterator begin() const noexcept {
            return iterator(entry_begin());
        }
        iterator end() const noexcept {
            return iterator(entry_end());
        }
        
        entry_iterator entry_begin() const noexcept {
            return entry_iterator(self_, data_.from);
        }
        entry_iterator entry_end() const noexcept {
            return entry_iterator(self_, data_.to);
        }
        
        index_type size() const noexcept
        {
            const auto diff = entry_end() - entry_begin();
            
            MEFDN_ASSERT(diff >= 0);
            
            return static_cast<index_type>(diff);
        }
        
    protected:
        void commit()
        {
            MEFDN_ASSERT(valid());
            self_ = nullptr;
        }
        
        derived_type*       self_;
        transaction_data    data_;
    };
    
public:
    void enqueue(const element_type& val)
    {
        auto first = &val;
        const auto last = first + 1;
        
        do {
            first = this->enqueue(first, last);
            // TODO: busy loop
        }
        while (first != last);
    }
    
    template <typename InputIterator>
    MEFDN_NODISCARD
    InputIterator enqueue(InputIterator first, const InputIterator last)
    {
        const auto num_req_signed = mefdn::distance(first, last);
        MEFDN_ASSERT(num_req_signed >= 0);
        
        const auto num_req = static_cast<index_type>(num_req_signed);
        
        auto t = derived().try_enqueue(num_req);
        
        if (MEFDN_UNLIKELY(!t.valid())) {
            // Failed to enqueue.
            return first;
        }
        
        auto t_itr = t.begin();
        const auto t_last = t.end();
        
        MEFDN_ASSERT(static_cast<index_type>(t_last - t_itr) <= num_req);
        
        index_type num_enqueued = 0;
        
        for ( ; t_itr != t_last; ++t_itr, ++first)
        {
            *t_itr = *first;
            MEFDN_ASSERT(first != last);
            
            ++num_enqueued;
        }
        
        MEFDN_ASSERT(num_enqueued <= num_req);
        
        t.commit(num_enqueued);
        
        return first;
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
    MEFDN_NODISCARD
    index_type dequeue(const index_type max_num, OutputIterator result)
    {
        auto t = derived().try_dequeue(max_num);
        
        if (MEFDN_UNLIKELY(!t.valid())) {
            return 0;
        }
        
        using mefdn::begin;
        using mefdn::end;
        
        auto t_itr = begin(t);
        const auto t_last = end(t);
        
        MEFDN_ASSERT(static_cast<index_type>(t_last - t_itr) <= max_num);
        
        index_type num_dequeued = 0;
        
        for ( ; t_itr != t_last; ++t_itr, ++result)
        {
            *result = *t_itr;
            
            ++num_dequeued;
        }
        
        MEFDN_ASSERT(num_dequeued <= max_num);
        
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
    char pad1_[MEFDN_CACHE_LINE_SIZE - sizeof(index_type)];
    
protected:
    atomic_type tail_;
    
private:
    char pad2_[MEFDN_CACHE_LINE_SIZE - sizeof(index_type)];
    
private:
    derived_type& derived() noexcept {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace detail

} // namespace mefdn
} // namespace menps

