
#pragma once

#include "mgcom.hpp"
#include <mgbase/type_traits.hpp>

namespace mgcom {

namespace typed_rma {

typedef std::ptrdiff_t  index_diff_t;

/**
 * Statically-sized buffer.
 *
 * This type must be an incomplete type to avoid calculating the size.
 */
template <index_t (*Size)()>
struct static_buffer;

/**
 * Statically-extended buffer.
 *
 * This type must be an incomplete type to avoid calculating the size.
 */
template <typename Header, index_t (*ExtendedSize)()>
struct static_extended_buffer;

/**
 * Dynamically-extended buffer.
 *
 * This type must be an incomplete type to avoid calculating the size.
 */
template <typename Header>
struct dynamic_extended_buffer;


template <typename T>
struct value_traits {
    static MGBASE_CONSTEXPR_FUNCTION index_t size() MGBASE_NOEXCEPT {
        return sizeof(T);
    }
};

template <index_t (*Size)()>
struct value_traits< static_buffer<Size> > {
    static MGBASE_CONSTEXPR_FUNCTION index_t size() MGBASE_NOEXCEPT {
        return Size();
    }
};


template <typename Header, index_t (*ExtendedSize)()>
struct value_traits< static_extended_buffer<Header, ExtendedSize> > {
    static MGBASE_CONSTEXPR_FUNCTION index_t size() MGBASE_NOEXCEPT {
        return sizeof(Header) + ExtendedSize();
    }
};


#if 0

template <typename From, typename To>
struct may_be_pointer_convertible
    : mgbase::is_convertible<From*, To*> { };

template <index_t (*FromSize)(), index_t (*ToSize)()>
struct may_be_pointer_convertible< static_buffer<FromSize>, static_buffer<ToSize> >
    : mgbase::true_type { };

template <index_t (*FromExtendedSize)(), index_t (*ToExtendedSize)()>
struct may_be_pointer_convertible< static_extended_buffer<FromExtendedSize>, static_extended_buffer<ToExtendedSize> >
    : mgbase::true_type { };



template <typename From, typename To>
struct conversion_traits
{
    static const bool may_be_convertible = mgbase::is_convertible<From*, To*>::value;
    
    static bool convertible() MGBASE_NOEXCEPT { return may_convertible; }
};

template <index_t (*ExtendedSize)(), typename To>
struct conversion_traits
{
    static const bool may_be_convertible = mgbase::is_convertible<From*, To*>::value;
    
    static bool convertible() MGBASE_NOEXCEPT { return may_convertible; }
};

// forward declarations

namespace detail {

template <template <typename> class Derived, typename T> class pointer_base;

}

template <typename T, template <typename> class Derived, typename U>
inline mgbase::enable_if<
    conversion_traits<U, T>::may_convertible
,   Derived<T>
>
static_pointer_cast(const detail::pointer_base<Derived, U>& ptr);

#endif

// definitions

namespace detail {


template <template <typename> class Derived, typename T>
class pointer_base_access
{
private:
    typedef Derived<T>  derived_type;
    
    template <typename U>
    static derived_type create(const U& value) { return derived_type(value); }
    
    template <template <typename> class Derived2, typename T2, bool IsCompound>
    friend class pointer_base_member;
    
    template <template <typename> class Derived2, typename T2>
    friend class pointer_base;
    
    #if 0
    template <typename T2, template <typename> class Derived2, typename U>
    friend mgbase::enable_if<
        mgbase::is_convertible<U*, T2*>::value
    ,   Derived<T2>
    >
    static_pointer_cast(const detail::pointer_base<Derived2, U>& ptr);
    #endif
};

template <template <typename> class Derived, typename T,
    bool IsCompound = mgbase::is_compound<typename mgbase::remove_cv<T>::type>::value>
class pointer_base_member { };

template <template <typename> class Derived, typename T>
class pointer_base_member<Derived, T, true>
{
    typedef pointer_base_access<Derived, T> access;
    
public:
    template <typename U>
    Derived<U> member(U (T::*q)) const {
        const index_t offset =
            reinterpret_cast<mgbase::uint8_t*>(&(static_cast<T*>(MGBASE_NULLPTR)->*q))
            - static_cast<mgbase::uint8_t*>(MGBASE_NULLPTR);
        
        return access::create(rma::advanced(derived().to_address(), offset));
    }
    
private:
          Derived<T>& derived()       { return static_cast<      Derived<T>&>(*this); }
    const Derived<T>& derived() const { return static_cast<const Derived<T>&>(*this); }
};

template <template <typename> class Derived, typename T>
class pointer_base
    : public pointer_base_member<Derived, T>
{
    typedef Derived<T>                      derived_type;
    typedef pointer_base_access<Derived, T> access;
    
public:
    
    derived_type operator + (index_diff_t diff) const MGBASE_NOEXCEPT {
        derived_type result = derived();
        result += diff;
        return result;
    }
    
    
    
private:
          Derived<T>& derived()       MGBASE_NOEXCEPT { return static_cast<      Derived<T>&>(*this); }
    const Derived<T>& derived() const MGBASE_NOEXCEPT { return static_cast<const Derived<T>&>(*this); }
};

}
#if 0

template <typename T, template <typename> class Derived, typename U>
inline mgbase::enable_if<
    mgbase::is_convertible<U*, T*>::value
,   Derived<T>
>
static_pointer_cast(const detail::pointer_base<Derived, U>& ptr) {
    
    return detail::pointer_base_access<Derived, T>::create(ptr.to_address());
}

#endif


template <typename T>
class remote_pointer
    : public detail::pointer_base<typed_rma::remote_pointer, T>
    //: public detail::remote_pointer_base<T>
{
    typedef detail::pointer_base<typed_rma::remote_pointer, T>  base;
    
public:
    remote_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ remote_pointer(const remote_pointer<U>&) MGBASE_NOEXCEPT;
    
    remote_pointer& operator += (index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::advanced(addr_, index * value_traits<T>::size());
        return *this;
    }
    
    // removed
    /*operator rma::remote_address() const MGBASE_NOEXCEPT {
        return to_address();
    }*/
    
    rma::remote_address to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
private:
    explicit remote_pointer(rma::remote_address addr)
        : addr_(addr) { }
    
    rma::remote_address addr_;
    
    template <template <typename> class Derived, typename U>
    friend class detail::pointer_base_access;
};

template <typename T>
class local_pointer
    : public detail::pointer_base<typed_rma::local_pointer, T>
{
    typedef detail::pointer_base<typed_rma::local_pointer, T>   base;
 
public:
    template <typename U>
    /*implicit*/ local_pointer(const local_pointer<U>&) MGBASE_NOEXCEPT;
    
    //operator local_pointer<const T>();
    local_pointer& operator += (index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::advanced(addr_, index * value_traits<T>::size());
        return *this;
    }
    
    operator T* () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    T* raw() const MGBASE_NOEXCEPT { return static_cast<T*>(rma::to_pointer(addr_)); }
    
    rma::local_address to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }

private:
    explicit local_pointer(rma::local_address addr)
        : addr_(addr) { }
    
    rma::local_address addr_;
    
    template <template <typename> class Derived, typename U>
    friend class detail::pointer_base_access;
};

namespace {

template <typename T>
inline void remote_read_nb(
    rma::remote_read_cb&            cb
,   process_id_t                    proc
,   const remote_pointer<const T>&  remote_ptr
,   const local_pointer<T>&         local_ptr
,   index_t                         number_of_elements
) {
    mgcom::rma::remote_read_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * value_traits<T>::size()
    );
}

template <typename T>
inline void remote_read_nb(
    rma::remote_read_cb&            cb
,   process_id_t                    proc
,   const remote_pointer<T>&        remote_ptr
,   const local_pointer<T>&         local_ptr
,   index_t                         number_of_elements
) {
    remote_read_nb(cb, proc, remote_pointer<const T>(remote_ptr), local_ptr, number_of_elements);
}

template <typename T>
inline void remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   remote_pointer<T>               remote_ptr
,   local_pointer<const T>          local_ptr
,   index_t                         number_of_elements
) {
    mgcom::rma::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * value_traits<T>::size()
    );
}

template <typename T>
inline void remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   remote_pointer<T>               remote_ptr
,   local_pointer<T>                local_ptr
,   index_t                         number_of_elements
) {
    mgcom::rma::remote_write_nb(cb, proc, remote_ptr, local_pointer<const T>(local_ptr), number_of_elements);
}

/**
 * Non-blocking local compare-and-swap.
 */
inline void local_compare_and_swap_default_nb(
    rma::local_compare_and_swap_default_cb&                 cb
,   const local_pointer<rma::atomic_default_t>&             target_ptr
,   const local_pointer<const rma::atomic_default_t>&       expected_ptr
,   const local_pointer<const rma::atomic_default_t>&       desired_ptr
,   const local_pointer<rma::atomic_default_t>&             result_ptr
) {
    mgcom::rma::local_compare_and_swap_default_nb(
        cb
    ,   target_ptr.to_address()
    ,   expected_ptr.to_address()
    ,   desired_ptr.to_address()
    ,   result_ptr.to_address()
    );
}

}

template <typename T>
class remote_dynamic_array
{
public:
    
    
    inline void append_nb(index_t index, const local_pointer<const T>&);
    
private:
    remote_pointer<mgcom::rma::atomic_default_t> pointer_size() const MGBASE_NOEXCEPT { return addr_; }
    remote_pointer<T> pointer_at() const MGBASE_NOEXCEPT { return mgcom::rma::advanced(addr_, sizeof(mgcom::rma::atomic_default_t)); }
    
    rma::remote_address addr_;
};

}

}

