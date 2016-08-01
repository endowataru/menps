
#pragma once

#include <mgbase/iterator.hpp>
#include <mgbase/utility/addressof.hpp>
#include <mgbase/type_traits/is_convertible.hpp>
#include <mgbase/type_traits/remove_const.hpp>

namespace mgbase {

template <typename I, typename V, typename CT, typename R, typename D>
class iterator_facade;

namespace detail {

template <
    typename I, typename V, typename CT, typename R, typename D
,   bool IsBidirectional
,   bool IsRandomAccess
>
class iterator_facade_base;

}  // namespace detail

class iterator_core_access
{
private:
    template <typename I, typename V, typename CT, typename R, typename D>
    friend class iterator_facade;
    
    template <typename I, typename V, typename CT, typename R, typename D, bool B1, bool B2>
    friend class detail::iterator_facade_base;
    
    template <typename Derived>
    static typename Derived::reference dereference(const Derived& d) {
        return d.dereference();
    }
    
    template <typename Derived>
    static void increment(Derived& d) {
        d.increment();
    }
    
    template <typename Derived>
    static void decrement(Derived& d) {
        d.decrement();
    }
    
    // TODO: different types
    template <typename Derived>
    static bool equal(const Derived& d1, const Derived& d2) {
        return d1.equal(d2);
    }
    
    template <typename Derived>
    static void advance(Derived& d, const typename Derived::difference_type n) {
        d.advance(n);
    }
    
    // TODO: different types
    template <typename Derived>
    static typename Derived::difference_type distance_from(const Derived& d1, const Derived& d2) {
        return -d1.distance_to(d2);
    }
};

namespace detail {

// not bidirectional, not random accessible
template <
    typename Derived
,   typename Value
,   typename CategoryOrTraversal
,   typename Reference
,   typename Difference
>
class iterator_facade_base<Derived, Value, CategoryOrTraversal, Reference, Difference, false, false>
{
public:
    typedef typename mgbase::remove_const<Value>::type  value_type;
    typedef Reference                                   reference;
    typedef Value*                                      pointer;
    typedef Difference                                  difference_type;
    
    reference operator * () const
    {
        return iterator_core_access::dereference(this->derived());
    }
    
    pointer operator -> () const
    {
        return mgbase::addressof(*this);
    }
    
    Derived& operator ++ ()
    {
        iterator_core_access::increment(this->derived());
        return this->derived();
    }
    
    // TODO: return type (see Boost's implementation)
    Derived operator ++ (int /*post increment*/)
    {
        Derived tmp(this->derived());
        ++*this;
        return tmp;
    }
    
    bool operator == (const Derived& other) const
    {
        return iterator_core_access::equal(this->derived(), other);
    }
    
    bool operator != (const Derived& other) const
    {
        return !(*this == other);
    }
    
protected:
    Derived& derived() MGBASE_NOEXCEPT {
        return static_cast<Derived&>(*this);
    }
    const Derived& derived() const MGBASE_NOEXCEPT {
        return static_cast<const Derived&>(*this);
    }
};

// bidirectional
template <
    typename Derived
,   typename Value
,   typename CategoryOrTraversal
,   typename Reference
,   typename Difference
>
class iterator_facade_base<Derived, Value, CategoryOrTraversal, Reference, Difference, true, false>
    : public iterator_facade_base<Derived, Value, CategoryOrTraversal, Reference, Difference, false, false>
{
public:
    Derived& operator -- ()
    {
        iterator_core_access::decrement(this->derived());
        return *this;
    }
    
    Derived operator -- (int)
    {
        Derived tmp(this->derived());
        --*this;
        return tmp;
    }
};

// random accessible
template <
    typename Derived
,   typename Value
,   typename CategoryOrTraversal
,   typename Reference
,   typename Difference
>
class iterator_facade_base<Derived, Value, CategoryOrTraversal, Reference, Difference, true, true>
    : public iterator_facade_base<Derived, Value, CategoryOrTraversal, Reference, Difference, true, false>
{
private:
    typedef iterator_facade_base<Derived, Value, CategoryOrTraversal, Reference, Difference, true, false>
        base;
    
    typedef typename base::reference        reference;
    typedef typename base::difference_type  difference_type;
    
public:
    reference operator [] (difference_type n) const
    {
        Derived tmp(this->derived());
        tmp += n;
        return *tmp;
    }
    
    Derived& operator += (difference_type n)
    {
        iterator_core_access::advance(this->derived(), n);
        return this->derived();
    }
    
    Derived& operator -= (difference_type n)
    {
        iterator_core_access::advance(this->derived(), -n);
        return this->derived();
    }
    
    Derived operator - (difference_type x) const
    {
        Derived tmp(this->derived());
        return tmp -= x;
    }
};

} // namespace detail

template <
    typename Derived
,   typename Value
,   typename CategoryOrTraversal
,   typename Reference  = Value&
,   typename Difference = mgbase::ptrdiff_t
>
class iterator_facade
    : public detail::iterator_facade_base<
        Derived
    ,   Value
    ,   CategoryOrTraversal
    ,   Reference
    ,   Difference
    ,   mgbase::is_convertible<CategoryOrTraversal, mgbase::bidirectional_iterator_tag>::value
    ,   mgbase::is_convertible<CategoryOrTraversal, mgbase::random_access_iterator_tag>::value
    >
    { };

} // namespace mgbase

