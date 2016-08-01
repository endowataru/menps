
#pragma once

#include <mgbase/iterator_facade.hpp>
#include <mgbase/type_traits/remove_cv.hpp>
#include <mgbase/assert.hpp>
#include <vector>

namespace mgbase {

namespace detail {

struct index_list_node
{
    index_list_node* prev;
    index_list_node* next;
};

template <typename Derived, typename T>
class index_list_iterator_base
    : public mgbase::iterator_facade<
        Derived
    ,   T
    ,   mgbase::input_iterator_tag
    ,   T
    >
{
    typedef typename mgbase::remove_cv<T>::type index_type;
    
public:
    // TODO: make these private
    index_list_node* arr() const MGBASE_NOEXCEPT { return arr_; }
    index_list_node* ptr() const MGBASE_NOEXCEPT { return ptr_; }
    
protected:
    index_list_iterator_base(index_list_node* const arr, index_list_node* const ptr)
        : arr_(arr)
        , ptr_(ptr)
    {
        MGBASE_ASSERT(arr_ != MGBASE_NULLPTR);
    }
    
private:
    friend class mgbase::iterator_core_access;
    
    index_type dereference() const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(ptr_ != MGBASE_NULLPTR); // not end()
        return static_cast<index_type>(ptr_ - arr_);
    }
    
    void increment() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(ptr_ != MGBASE_NULLPTR); // not end()
        ptr_ = ptr_->next;
    }
    void decrement() MGBASE_NOEXCEPT {
        MGBASE_ASSERT(ptr_ != MGBASE_NULLPTR); // not end()
        ptr_ = ptr_->prev;
    }
    
    bool equal(const Derived& other) const MGBASE_NOEXCEPT {
        MGBASE_ASSERT(arr_ == other.arr_);
        return ptr_ == other.ptr_;
    }
    
    index_list_node* arr_;
    index_list_node* ptr_;
};

} // namespace detail

template <typename T>
class index_list
{
    typedef detail::index_list_node node_type;
    
public:
    class iterator
        : public detail::index_list_iterator_base<iterator, T>
    {
    private:
        typedef detail::index_list_iterator_base<iterator, T>   base;
        
        friend class index_list<T>;
        
        iterator(node_type* const arr, node_type* const ptr)
            : base(arr, ptr) { }
    };
    
    typedef iterator    const_iterator;
    
    explicit index_list(const mgbase::size_t size)
        : arr_(size, node_type())
        , head_{MGBASE_NULLPTR, &tail_}
        , tail_{&head_, MGBASE_NULLPTR}
        { }
    
    index_list(const index_list&) = delete;
    
    index_list& operator = (const index_list&) = delete;
    
    iterator begin() const MGBASE_NOEXCEPT {
        return iterator(arr_.data(), head_.next);
    }
    iterator cbegin() const MGBASE_NOEXCEPT {
        return begin();
    }
    iterator end() const MGBASE_NOEXCEPT {
        return iterator(arr_.data(), &tail_);
    }
    iterator cend() const MGBASE_NOEXCEPT {
        return end();
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        return begin() == end();
    }
    
    void push_front(const T index)
    {
        insert(begin(), index);
    }
    void push_back(const T index)
    {
        insert(end(), index);
    }
    
    void pop_front()
    {
        MGBASE_ASSERT(!empty());
        erase(begin());
    }
    void pop_back()
    {
        MGBASE_ASSERT(!empty());
        erase(mgbase::prev(end()));
    }
    
    iterator insert(const iterator itr, const T index) // insert_before
    {
        MGBASE_ASSERT(0 <= index);
        MGBASE_ASSERT(static_cast<mgbase::size_t>(index) < arr_.size());
        
        auto& curr = arr_[static_cast<mgbase::size_t>(index)];
        auto& next = *itr.ptr();
        auto& prev = *next.prev;
        
        MGBASE_ASSERT(curr.prev == MGBASE_NULLPTR);
        MGBASE_ASSERT(curr.next == MGBASE_NULLPTR);
        
        curr.prev = &prev;
        curr.next = &next;
        
        prev.next = &curr;
        next.prev = &curr;
        
        return iterator(arr_.data(), &curr);
    }
    
    iterator erase(const iterator itr)
    {
        auto& curr = *itr.ptr();
        MGBASE_ASSERT(curr.prev != MGBASE_NULLPTR);
        MGBASE_ASSERT(curr.next != MGBASE_NULLPTR);
        
        auto& prev = *curr.prev;
        auto& next = *curr.next;
        
        prev.next = &next;
        next.prev = &prev;
        
        curr.prev = MGBASE_NULLPTR;
        curr.next = MGBASE_NULLPTR;
        
        return iterator(arr_.data(), &next);
    }
    
    bool exists(const T index)
    {
        auto& elem = arr_[static_cast<mgbase::size_t>(index)];
        MGBASE_ASSERT((elem.prev == MGBASE_NULLPTR) == (elem.next == MGBASE_NULLPTR));
        return elem.prev != MGBASE_NULLPTR;
    }
    
private:
    mutable std::vector<node_type>  arr_; // TODO
    node_type                       head_;
    mutable node_type               tail_; // TODO
};

} // namespace mgbase

