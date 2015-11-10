
#pragma once

#include "mgcom.hpp"
#include <mgbase/type_traits.hpp>
#include <mgbase/pointer_facade.hpp>
#include <mgbase/runtime_sized.hpp>

namespace mgcom {

namespace typed_rma {

#if 0

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

/**
 * If the type From is convertible to the type T, "value" is true.
 */
template <typename From, typename To>
struct pointer_convertible
    : mgbase::is_convertible<From*, To*> { };

template <typename FromHeader, index_t (*FromExtendedSize)(), typename To>
struct pointer_convertible<static_extended_buffer<FromHeader, FromExtendedSize>, To>
    : mgbase::integral_constant<bool,
        pointer_convertible<FromHeader, To>::value
        || mgbase::is_convertible<static_extended_buffer<FromHeader, FromExtendedSize>*, To*>::value
    > { };

template <typename FromHeader, index_t (*FromExtendedSize)(), typename To>
struct pointer_convertible<const static_extended_buffer<FromHeader, FromExtendedSize>, To>  
    : mgbase::integral_constant<bool,
        pointer_convertible<const FromHeader, To>::value
        || mgbase::is_convertible<const static_extended_buffer<FromHeader, FromExtendedSize>*, To*>::value
    > { };


template <typename From, typename To>
struct assignable_to
    : mgbase::integral_constant<bool,
        // From* is convertible to To*
        pointer_convertible<typename mgbase::remove_const<From>::type, To>::value
        // To is not const
        && !mgbase::is_const<To>::value
    > { };

#endif

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

template <template <typename> class Derived, typename T> class pointer_facade;

}

template <typename T, template <typename> class Derived, typename U>
inline mgbase::enable_if<
    conversion_traits<U, T>::may_convertible
,   Derived<T>
>
static_pointer_cast(const detail::pointer_facade<Derived, U>& ptr);

#endif

// definitions
#if 0

namespace detail {

template <template <typename> class Derived, typename T>
class pointer_facade;

}

template <typename To, template <typename> class Derived, typename From>
inline
typename mgbase::enable_if<
    pointer_convertible<From, To>::value
,   Derived<To>
>::type
implicit_pointer_cast(const detail::pointer_facade<Derived, From>&);

namespace detail {

template <template <typename> class Derived, typename T>
class pointer_facade_access
{
private:
    typedef Derived<T>                              derived_type;
    typedef typename derived_type::address_type     address_type;
    
    static derived_type create(const address_type& addr) {
        return derived_type::create(addr);
    }
    
    template <typename U>
    static derived_type cast_from(const detail::pointer_facade<Derived, U>& ptr) {
        return create(static_cast<const Derived<U>&>(ptr).to_address());
    }
    
    template <template <typename> class Derived2, typename T2, bool HasMembers>
    friend class pointer_facade_member;
    
    template <template <typename> class Derived2, typename T2>
    friend class pointer_facade;
    
    template <typename To, template <typename> class Derived2, typename From>
    friend typename mgbase::enable_if<
        pointer_convertible<From, To>::value
    ,   Derived2<To>
    >::type
    typed_rma::implicit_pointer_cast(const detail::pointer_facade<Derived2, From>&);
};

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
    typedef pointer_facade_access<Derived, T> access;
    
public:
    template <typename U>
    Derived<U> member(U (T::*q)) const {
        const index_t offset =
            reinterpret_cast<mgbase::uint8_t*>(&(static_cast<T*>(MGBASE_NULLPTR)->*q))
            - static_cast<mgbase::uint8_t*>(MGBASE_NULLPTR);
        
        return access::create(rma::advanced(derived().to_address(), offset));
    }
    
private:
          derived_type& derived()       { return static_cast<      derived_type&>(*this); }
    const derived_type& derived() const { return static_cast<const derived_type&>(*this); }
};

template <template <typename> class Derived, typename T>
class pointer_facade_operators
{
    typedef Derived<T>                      derived_type;
    typedef pointer_facade_access<Derived, T> access;
    
public:
    derived_type operator + (index_diff_t diff) const MGBASE_NOEXCEPT {
        derived_type result = derived();
        result += diff;
        return result;
    }

private:
          derived_type& derived()       MGBASE_NOEXCEPT { return static_cast<      derived_type&>(*this); }
    const derived_type& derived() const MGBASE_NOEXCEPT { return static_cast<const derived_type&>(*this); }
};

template <template <typename> class Derived, typename T, std::size_t Size>
class pointer_facade_operators<Derived, T [Size]>
{
    typedef Derived<T[]>                    derived_type;
    typedef Derived<T>                      normal_pointer_type;
    typedef pointer_facade_access<Derived, T> access;
    
public:
    normal_pointer_type operator + (index_diff_t diff) const MGBASE_NOEXCEPT {
        normal_pointer_type result = derived();
        result += diff;
        return result;
    }

private:
          derived_type& derived()       MGBASE_NOEXCEPT { return static_cast<      derived_type&>(*this); }
    const derived_type& derived() const MGBASE_NOEXCEPT { return static_cast<const derived_type&>(*this); }
};

template <template <typename> class Derived, typename T>
class pointer_facade
    : public pointer_facade_member<Derived, T>
    , public pointer_facade_operators<Derived, T>
    { };

}

template <typename To, template <typename> class Derived, typename From>
inline
typename mgbase::enable_if<
    pointer_convertible<From, To>::value
,   Derived<To>
>::type
implicit_pointer_cast(const detail::pointer_facade<Derived, From>& ptr)
{
    return detail::pointer_facade_access<Derived, To>::cast_from(ptr);
}

#endif

template <typename T>
class remote_pointer
    : public mgbase::pointer_facade<typed_rma::remote_pointer, T>
{
    typedef mgbase::pointer_facade<typed_rma::remote_pointer, T>  base;
    
public:
    typedef rma::remote_address     address_type;
    
#if MGBASE_CPP11_SUPPORTED
    remote_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ remote_pointer(const remote_pointer<U>&) MGBASE_NOEXCEPT;
#endif
    
    /*remote_pointer& operator += (index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::advanced(addr_, index * value_traits<T>::size());
        return *this;
    }*/
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
    static remote_pointer cast_from(const rma::remote_address& addr) {
        remote_pointer result;
        result.addr_ = addr;
        return result;
    }
    
private:
#if MGBASE_CPP11_SUPPORTED
    explicit remote_pointer(rma::remote_address addr)
        : addr_(addr) { }
#endif
    
    friend class mgbase::pointer_core_access;
    
    /*static remote_pointer create(const address_type& addr) {
        remote_pointer result;
        result.addr_ = addr;
        return result;
    }*/
    
    template <typename U>
    remote_pointer<U> cast_to() const MGBASE_NOEXCEPT {
        return remote_pointer<U>::cast_from(to_address());
    }
    
    void advance(index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::advanced(addr_, index * mgbase::runtime_size_of<T>());
    }
    
    address_type addr_;
};

template <typename T>
class local_pointer
    : public mgbase::pointer_facade<typed_rma::local_pointer, T>
{
    typedef mgbase::pointer_facade<typed_rma::local_pointer, T>   base;

public:
    typedef rma::local_address      address_type;
    
#if MGBASE_CPP11_SUPPORTED
    local_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ local_pointer(const local_pointer<U>&) MGBASE_NOEXCEPT;
#endif
    
    /*local_pointer& operator += (index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::advanced(addr_, index * value_traits<T>::size());
        return *this;
    }*/
    
    operator T* () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    T* raw() const MGBASE_NOEXCEPT { return static_cast<T*>(rma::to_pointer(addr_)); }
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }

private:
#if MGBASE_CPP11_SUPPORTED
    explicit local_pointer(rma::local_address addr)
        : addr_(addr) { }
#endif
    
    friend class mgbase::pointer_core_access;
    
    /*static local_pointer create(const address_type& addr) {
        local_pointer result;
        result.addr_ = addr;
        return result;
    }*/
    
    address_type addr_;
};


template <typename From, typename To>
struct assignable_to
    : mgbase::integral_constant<bool,
        // From* is convertible to To*
        mgbase::pointer_convertible<typename mgbase::remove_const<From>::type, To>::value
        // To is not const
        && !mgbase::is_const<To>::value
    > { };

namespace {

/// Simple remote read.
template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    assignable_to<Remote, Local>::value
>::type
remote_read_nb(
    rma::remote_read_cb&            cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
) {
    mgcom::rma::remote_read_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   mgbase::runtime_size_of<Remote>()
    );
}

/// Simple remote write.
template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    assignable_to<Local, Remote>::value
>::type
remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
) {
    mgcom::rma::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   mgbase::runtime_size_of<Local>()
    );
}

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
    ,   number_of_elements * mgbase::runtime_size_of<T>()
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
    remote_read_nb(
        cb
    ,   proc
    ,   mgbase::implicit_pointer_cast<const T>(remote_ptr)
    ,   local_ptr
    ,   number_of_elements
    );
}

template <typename T>
inline void remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   const remote_pointer<T>&        remote_ptr
,   const local_pointer<const T>&   local_ptr
,   index_t                         number_of_elements
) {
    mgcom::rma::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<T>()
    );
}

template <typename T>
inline void remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   const remote_pointer<T>&        remote_ptr
,   const local_pointer<T>&         local_ptr
,   index_t                         number_of_elements
) {
    mgcom::rma::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr
    ,   mgbase::implicit_pointer_cast<const T>(local_ptr)
    ,   number_of_elements
    );
}


inline void remote_atomic_read_default_nb(
    rma::remote_atomic_read_default_cb&                     cb
,   process_id_t                                            proc
,   const remote_pointer<const rma::atomic_default_t>&      remote_ptr
,   const local_pointer<rma::atomic_default_t>&             local_ptr
,   const local_pointer<rma::atomic_default_t>&             buf_ptr
) {
    mgcom::rma::remote_atomic_read_default_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   buf_ptr.to_address()
    );
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

/**
 * Non-blocking remote compare-and-swap.
 */
inline void remote_compare_and_swap_default_nb(
    rma::remote_compare_and_swap_default_cb&                cb
,   process_id_t                                            target_proc
,   const remote_pointer<rma::atomic_default_t>&            target_ptr
,   const local_pointer<const rma::atomic_default_t>&       expected_ptr
,   const local_pointer<const rma::atomic_default_t>&       desired_ptr
,   const local_pointer<rma::atomic_default_t>&             result_ptr
) {
    mgcom::rma::remote_compare_and_swap_default_nb(
        cb
    ,   target_proc
    ,   target_ptr.to_address()
    ,   expected_ptr.to_address()
    ,   desired_ptr.to_address()
    ,   result_ptr.to_address()
    );
}

/**
 * Non-blocking remote fetch-and-add.
 */
inline void remote_fetch_and_add_default_nb(
    rma::remote_fetch_and_add_default_cb&               cb
,   process_id_t                                        target_proc
,   const remote_pointer<rma::atomic_default_t>&        target_ptr
,   const local_pointer<const rma::atomic_default_t>&   value_ptr
,   const local_pointer<rma::atomic_default_t>&         result_ptr
) {
    mgcom::rma::remote_fetch_and_add_default_nb(
        cb
    ,   target_proc
    ,   target_ptr.to_address()
    ,   value_ptr.to_address()
    ,   result_ptr.to_address()
    );
}


template <typename T>
inline remote_pointer<T> use_remote_pointer(process_id_t proc_id, const local_pointer<T>& ptr) {
    return remote_pointer<T>::cast_from(mgcom::rma::use_remote_address(proc_id, ptr.to_address()));
}

}

}

}

