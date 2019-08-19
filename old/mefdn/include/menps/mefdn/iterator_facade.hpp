
#pragma once

#include <menps/mefdn/iterator.hpp>
#include <menps/mefdn/memory.hpp>
#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mefdn {

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
    typedef typename mefdn::remove_const<Value>::type  value_type;
    typedef Reference                                   reference;
    typedef Value*                                      pointer;
    typedef Difference                                  difference_type;
    
    reference operator * () const
    {
        return iterator_core_access::dereference(this->derived());
    }
    
    pointer operator -> () const
    {
        return mefdn::addressof(**this);
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
    Derived& derived() noexcept {
        return static_cast<Derived&>(*this);
    }
    const Derived& derived() const noexcept {
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
    
public:
    typedef typename base::reference        reference;
    typedef typename base::difference_type  difference_type;
    
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
    
    difference_type operator - (const Derived& other) const
    {
        return iterator_core_access::distance_from(this->derived(), other);
    }
};

} // namespace detail

template <
    typename Derived
,   typename Value
,   typename CategoryOrTraversal
,   typename Reference  = Value&
,   typename Difference = mefdn::ptrdiff_t
>
class iterator_facade
    : public detail::iterator_facade_base<
        Derived
    ,   Value
    ,   CategoryOrTraversal
    ,   Reference
    ,   Difference
    ,   mefdn::is_convertible<CategoryOrTraversal, mefdn::bidirectional_iterator_tag>::value
    ,   mefdn::is_convertible<CategoryOrTraversal, mefdn::random_access_iterator_tag>::value
    >
    { };

} // namespace mefdn
} // namespace menps

