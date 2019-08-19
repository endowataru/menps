
#pragma once

#include <menps/mefdn/iterator_facade.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/assert.hpp>
#include <vector>

namespace menps {
namespace mefdn {

namespace detail {

struct index_list_node
{
    index_list_node* prev;
    index_list_node* next;
};

template <typename Derived, typename T>
class index_list_iterator_base
    : public mefdn::iterator_facade<
        Derived
    ,   T
    ,   mefdn::input_iterator_tag
    ,   T
    >
{
    typedef typename mefdn::remove_cv<T>::type index_type;
    
public:
    // TODO: make these private
    index_list_node* arr() const noexcept { return arr_; }
    index_list_node* ptr() const noexcept { return ptr_; }
    
protected:
    index_list_iterator_base(index_list_node* const arr, index_list_node* const ptr)
        : arr_(arr)
        , ptr_(ptr)
    {
        MEFDN_ASSERT(arr_ != nullptr);
    }
    
private:
    friend class mefdn::iterator_core_access;
    
    index_type dereference() const noexcept {
        MEFDN_ASSERT(ptr_ != nullptr); // not end()
        return static_cast<index_type>(ptr_ - arr_);
    }
    
    void increment() noexcept {
        MEFDN_ASSERT(ptr_ != nullptr); // not end()
        ptr_ = ptr_->next;
    }
    void decrement() noexcept {
        MEFDN_ASSERT(ptr_ != nullptr); // not end()
        ptr_ = ptr_->prev;
    }
    
    bool equal(const Derived& other) const noexcept {
        MEFDN_ASSERT(arr_ == other.arr_);
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
    
    explicit index_list(const mefdn::size_t size)
        : arr_(size, node_type())
        , head_{nullptr, &tail_}
        , tail_{&head_, nullptr}
        { }
    
    index_list(const index_list&) = delete;
    
    index_list& operator = (const index_list&) = delete;
    
    iterator begin() const noexcept {
        return iterator(arr_.data(), head_.next);
    }
    iterator cbegin() const noexcept {
        return begin();
    }
    iterator end() const noexcept {
        return iterator(arr_.data(), &tail_);
    }
    iterator cend() const noexcept {
        return end();
    }
    
    bool empty() const noexcept {
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
        MEFDN_ASSERT(!empty());
        erase(begin());
    }
    void pop_back()
    {
        MEFDN_ASSERT(!empty());
        erase(mefdn::prev(end()));
    }
    
    iterator insert(const iterator itr, const T index) // insert_before
    {
        //MEFDN_ASSERT(0 <= index);
        MEFDN_ASSERT(static_cast<mefdn::size_t>(index) < arr_.size());
        
        auto& curr = arr_[static_cast<mefdn::size_t>(index)];
        auto& next = *itr.ptr();
        auto& prev = *next.prev;
        
        MEFDN_ASSERT(curr.prev == nullptr);
        MEFDN_ASSERT(curr.next == nullptr);
        
        curr.prev = &prev;
        curr.next = &next;
        
        prev.next = &curr;
        next.prev = &curr;
        
        return iterator(arr_.data(), &curr);
    }
    
    iterator erase(const iterator itr)
    {
        auto& curr = *itr.ptr();
        MEFDN_ASSERT(curr.prev != nullptr);
        MEFDN_ASSERT(curr.next != nullptr);
        
        auto& prev = *curr.prev;
        auto& next = *curr.next;
        
        prev.next = &next;
        next.prev = &prev;
        
        curr.prev = nullptr;
        curr.next = nullptr;
        
        return iterator(arr_.data(), &next);
    }
    
    bool exists(const T index)
    {
        auto& elem = arr_[static_cast<mefdn::size_t>(index)];
        MEFDN_ASSERT((elem.prev == nullptr) == (elem.next == nullptr));
        return elem.prev != nullptr;
    }
    
private:
    mutable std::vector<node_type>  arr_; // TODO
    node_type                       head_;
    mutable node_type               tail_; // TODO
};

} // namespace mefdn
} // namespace menps

