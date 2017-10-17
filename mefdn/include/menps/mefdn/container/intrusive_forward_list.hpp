
#pragma once

#include <menps/mefdn/iterator_facade.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace mefdn {

struct intrusive_forward_list_node_base
{
    intrusive_forward_list_node_base*   next;
};

template <typename T>
struct intrusive_forward_list_traits
{
    typedef intrusive_forward_list_node_base    node_base_type;
    typedef node_base_type*                     node_pointer_type;
     
    typedef T&                                  reference;
    typedef const T&                            const_reference;
    typedef T                                   value_type;
    typedef T*                                  pointer_type;
    typedef const T*                            const_pointer_type;
    
    static node_pointer_type get_next(const node_base_type& x) noexcept {
        return x.next;
    }
    static void set_next(node_base_type& x, const node_pointer_type next) noexcept {
        x.next = next;
    }
    
    static reference from_base(const node_pointer_type x) noexcept {
        MEFDN_ASSERT(x != nullptr);
        return static_cast<reference>(*x);
    }
    static node_pointer_type to_base(reference x) noexcept {
        return mefdn::addressof(x);
    }
};

namespace detail {

template <typename Derived, typename T>
class intrusive_forward_list_iterator_base
    : public mefdn::iterator_facade<
        Derived
    ,   typename intrusive_forward_list_traits<T>::value_type
    ,   mefdn::forward_iterator_tag
    ,   typename intrusive_forward_list_traits<T>::reference
    >
{
    typedef typename mefdn::remove_cv<T>::type elem_type;
    
    typedef intrusive_forward_list_traits<elem_type>    traits_type;
    typedef typename traits_type::node_base_type        node_base_type;
    typedef typename traits_type::node_pointer_type     node_pointer_type;
    
    typedef typename traits_type::reference             reference;
    
public:
    intrusive_forward_list_iterator_base(const intrusive_forward_list_iterator_base&) noexcept = default;
    
    intrusive_forward_list_iterator_base& operator = (const intrusive_forward_list_iterator_base&) noexcept = default;
    
protected:
    explicit intrusive_forward_list_iterator_base(const node_pointer_type curr)
        : curr_(curr)
        { }
    
private:
    friend class mefdn::iterator_core_access;
    
    reference dereference() const noexcept {
        return traits_type::from_base(curr_);
    }
    
    void increment() noexcept {
        MEFDN_ASSERT(curr_ != nullptr);
        curr_ = traits_type::get_next(*curr_);
    }
    
    bool equal(const Derived& other) const noexcept {
        return curr_ == other.curr_;
    }
    
    node_pointer_type curr_;
};

} // namespace detail



template <typename T>
class intrusive_forward_list
{
    typedef intrusive_forward_list_traits<T>        traits_type;
    
    typedef typename traits_type::node_base_type        node_base_type;
    typedef typename traits_type::node_pointer_type     node_pointer_type;
    
public:
    class iterator
        : public detail::intrusive_forward_list_iterator_base<iterator, T>
    {
    public:
        iterator(const iterator&) noexcept = default;
        
        iterator& operator = (const iterator&) noexcept = default;
        
    private:
        typedef detail::intrusive_forward_list_iterator_base<iterator, T>  base;
        
        friend class intrusive_forward_list<T>;
        
        explicit iterator(const node_pointer_type curr)
            : base(curr) { }
    };
    
    #if 0
    class const_iterator
        : public detail::intrusive_forward_list_iterator_base<const_iterator, const T>
    {
    public:
        /*implicit*/ const_iterator(const iterator& other)
            : base(other->operator->()) { }
        
        const_iterator(const const_iterator&) noexcept = default;
        
        const_iterator& operator = (const const_iterator&) noexcept = default;
     
    private:
        typedef detail::intrusive_forward_list_iterator_base<const_iterator, const T>  base;
        
        friend class intrusive_forward_list<T>;
        
        explicit const_iterator(const node_base_type* const curr)
            : base(curr) { }
    };
    #endif
    
    // Transfer types from traits
    typedef typename traits_type::reference             reference;
    typedef typename traits_type::const_reference       const_reference;
    typedef typename traits_type::value_type            value_type;
    typedef typename traits_type::pointer_type          pointer_type;
    typedef typename traits_type::const_pointer_type    const_pointer_type;
    
    // These members are less important to be customized
    typedef mefdn::size_t      size_type;
    typedef mefdn::ptrdiff_t   difference_type;
    
    intrusive_forward_list()
        : head_()
        { }
    
    intrusive_forward_list(const intrusive_forward_list&) = delete;
    
    intrusive_forward_list& operator = (const intrusive_forward_list&) = delete;
    
    #if 0
    void push_back(T& elem)
    {
        auto next = mefdn::addressof(elem);
        
        const auto last = last_;
        set_next(*last, next);
        set_next(*next, nullptr);
    }
    #endif
    
    bool empty() /*const*/ noexcept {
        return begin() == end();
    }
    
    void push_front(reference elem) {
        const auto next = get_next(head_);
        set_next(elem, next);
        
        set_next(head_, to_base(elem));
    }
    void pop_front() {
        MEFDN_ASSERT(!empty());
        
        const auto next = get_next(head_);
        const auto next_next = get_next(*next);
        set_next(head_, next_next);
    }
    
    reference front() {
        return *begin();
    }
    const reference front() const {
        return *begin();
    }
    
    iterator before_begin() {
        return iterator(mefdn::addressof(head_));
    }
    iterator begin() {
        return ++before_begin();
    }
    iterator end() {
        return iterator(nullptr);
    }
    
private:
    static node_pointer_type get_next(const node_base_type& elem) noexcept {
        return traits_type::get_next(elem);
    }
    static void set_next(node_base_type& elem, const node_pointer_type next) {
        traits_type::set_next(elem, next);
    }
    static node_pointer_type to_base(reference elem) {
        return traits_type::to_base(elem);
    }
    
    node_base_type head_;
};

template <typename T>
inline typename intrusive_forward_list<T>::iterator begin(intrusive_forward_list<T>& self) {
    return self.begin();
}
template <typename T>
inline typename intrusive_forward_list<T>::iterator end(intrusive_forward_list<T>& self) {
    return self.end();
}

} // namespace mefdn
} // namespace menps

