
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/type_traits.hpp>

#include <mgbase/renamed_type.hpp>

#include <cstring>

namespace mgbase {

namespace detail {

/**
 * If the type From is convertible to the type T, "value" is true.
 */
template <typename From, typename To>
struct implicit_pointer_castable
    : mgbase::is_convertible<
        typename get_actual_type<From>::type*
    ,   typename get_actual_type<To>::type*
    > { };

/*
 * If the type From is convertible to the type T, "value" is true.
 */
template <typename From, typename To>
struct reinterpret_pointer_castable
    : mgbase::integral_constant<bool,
        !(mgbase::is_const<From>::value && !mgbase::is_const<To>::value)
    > { };

}

// declarations

template <template <typename> class Derived, typename T>
class pointer_facade;

template <typename To, template <typename> class Derived, typename From>
inline typename mgbase::enable_if<
    detail::implicit_pointer_castable<From, To>::value
,   Derived<To>
>::type
implicit_pointer_cast(const pointer_facade<Derived, From>&);

template <typename To, template <typename> class Derived, typename From>
inline typename mgbase::enable_if<
    detail::reinterpret_pointer_castable<From, To>::value
,   Derived<To>
>::type
reinterpret_pointer_cast(const pointer_facade<Derived, From>& ptr);

namespace detail
{
    template <template <typename> class Derived, typename T, bool HasMembers>
    class pointer_facade_member;
    
    template <template <typename> class Derived, typename T>
    class pointer_facade_operators;
}

/// Transfer class for private members of pointer_facade

class pointer_core_access
{
private:
    template <template <typename> class Derived, typename T>
    static Derived<T>& derived(pointer_facade<Derived, T>& ptr) {
        return static_cast<Derived<T>&>(ptr);
    }
    
    template <template <typename> class Derived, typename T>
    static const Derived<T>& derived(const pointer_facade<Derived, T>& ptr) {
        return static_cast<const Derived<T>&>(ptr);
    }
    
    template <typename U, template <typename> class Derived, typename T>
    static Derived<U> cast_to(const pointer_facade<Derived, T>& ptr) {
        return derived(ptr).template cast_to<U>();
    }
    
    template <template <typename> class Derived, typename T>
    static void advance(pointer_facade<Derived, T>& ptr, std::ptrdiff_t diff) {
        derived(ptr).advance(diff);
    }
    
    template <template <typename> class Derived, typename T>
    static bool is_null(const pointer_facade<Derived, T>& ptr) {
        return derived(ptr).is_null();
    }
    
    template <template <typename> class Derived, typename T, bool HasMembers>
    friend class detail::pointer_facade_member;
    
    template <template <typename> class Derived, typename T>
    friend class detail::pointer_facade_operators;
    
    template <template <typename> class Derived, typename T>
    friend class pointer_facade;
    
    template <typename To, template <typename> class Derived, typename From>
    friend typename mgbase::enable_if<
        detail::implicit_pointer_castable<From, To>::value
    ,   Derived<To>
    >::type
    implicit_pointer_cast(const pointer_facade<Derived, From>& ptr);
    
    template <typename To, template <typename> class Derived, typename From>
    friend typename mgbase::enable_if<
        detail::reinterpret_pointer_castable<From, To>::value
    ,   Derived<To>
    >::type
    reinterpret_pointer_cast(const pointer_facade<Derived, From>& ptr);
};

template <typename To, template <typename> class Derived, typename From>
inline typename mgbase::enable_if<
    detail::implicit_pointer_castable<From, To>::value
,   Derived<To>
>::type
implicit_pointer_cast(const pointer_facade<Derived, From>& ptr) {
    return pointer_core_access::cast_to<To>(ptr);
}

template <typename To, template <typename> class Derived, typename From>
inline typename mgbase::enable_if<
    detail::reinterpret_pointer_castable<From, To>::value
,   Derived<To>
>::type
reinterpret_pointer_cast(const pointer_facade<Derived, From>& ptr) {
    return pointer_core_access::cast_to<To>(ptr);
}

namespace detail {

template <template <typename> class Derived, typename T,
    bool HasMembers =
        mgbase::is_compound<typename mgbase::remove_cv<T>::type>::value
        && !mgbase::is_array<typename mgbase::remove_cv<T>::type>::value
>
class pointer_facade_member { };

template <template <typename> class Derived, typename T>
class pointer_facade_member<Derived, T, true>
{
    typedef Derived<T>                      derived_type;
    
public:
    template <typename U>
    Derived<U> member(U (T::*q)) const {
        // Calculate the offset of the member q in the struct T.
        const std::size_t offset =
            reinterpret_cast<const mgbase::uint8_t*>(&(static_cast<T*>(MGBASE_NULLPTR)->*q))
            - static_cast<const mgbase::uint8_t*>(MGBASE_NULLPTR);
        
        Derived<mgbase::uint8_t> byte_ptr = pointer_core_access::cast_to<mgbase::uint8_t>(derived());
        pointer_core_access::advance(byte_ptr, offset);
        return pointer_core_access::cast_to<U>(byte_ptr);
    }
    
private:
          derived_type& derived()       { return static_cast<      derived_type&>(*this); }
    const derived_type& derived() const { return static_cast<const derived_type&>(*this); }
};

template <template <typename> class Derived, typename T>
class pointer_facade_operators
{
    typedef Derived<T>                      derived_type;
    
public:
    derived_type operator += (std::ptrdiff_t diff) {
        pointer_core_access::advance(derived(), diff);
        return derived();
    }
    
    derived_type operator + (std::ptrdiff_t diff) const {
        derived_type result = derived();
        result += diff;
        return result;
    }

private:
          derived_type& derived()       MGBASE_NOEXCEPT { return static_cast<      derived_type&>(*this); }
    const derived_type& derived() const MGBASE_NOEXCEPT { return static_cast<const derived_type&>(*this); }
};

}

template <template <typename> class Derived, typename T>
class pointer_facade
    : public detail::pointer_facade_member<Derived, T>
    , public detail::pointer_facade_operators<Derived, T>
{
public:
    operator Derived<const T>() const {
        return implicit_pointer_cast<const T>(*this);
    }
    
    bool operator == (mgbase::nullptr_t) const {
        return pointer_core_access::is_null(*this);
    }
};

template <template <typename> class Derived, typename T>
inline bool operator != (const pointer_facade<Derived, T>& ptr, mgbase::nullptr_t) {
    return !(ptr == MGBASE_NULLPTR);
}

} // namespace mgbase

