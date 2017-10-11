
#pragma once

#include <mgcom/rma/address.hpp>

#include <mgbase/type_traits.hpp>
#include <mgbase/pointer_facade.hpp>
#include <mgbase/runtime_sized.hpp>

namespace mgcom {
namespace rma {

template <typename T>
class local_ptr;

namespace detail {

template <
    typename T
,   bool Dereferencable = ! mgbase::is_void<T>::value
>
class local_ptr_deref { };

template <typename T>
class local_ptr_deref<T, true>
{
    typedef local_ptr<T>    derived_type;
    
public:
    T* operator -> () const MGBASE_NOEXCEPT {
        return derived().raw();
    }
    T& operator * () const MGBASE_NOEXCEPT {
        return *derived().raw();
    }
    
private:
    const derived_type& derived() const MGBASE_NOEXCEPT {
        return static_cast<const derived_type&>(*this);
    }
};

} // namespace detail

template <typename T>
class local_ptr
    : public mgbase::pointer_facade<rma::local_ptr, T>
    , public detail::local_ptr_deref<T>
{
    typedef mgbase::pointer_facade<rma::local_ptr, T>   base;

public:
    typedef untyped::local_address      address_type;
    
#ifdef MGBASE_CXX11_SUPPORTED
    local_ptr() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ local_ptr(const local_ptr<U>&) MGBASE_NOEXCEPT;
#endif
    operator T* () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    /*T* operator -> () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    T& operator * () const MGBASE_NOEXCEPT {
        return *raw();
    }*/
    
    T* raw() const MGBASE_NOEXCEPT { return static_cast<T*>(untyped::to_raw_pointer(addr_)); }
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
    static local_ptr cast_from(const address_type& addr) {
        local_ptr result;
        result.addr_ = addr;
        return result;
    }

private:
#ifdef MGBASE_CXX11_SUPPORTED
    explicit local_ptr(address_type addr)
        : addr_(addr) { }
#endif
    
    friend class mgbase::pointer_core_access;
    
    template <typename U>
    local_ptr<U> cast_to() const MGBASE_NOEXCEPT {
        return local_ptr<U>::cast_from(to_address());
    }
    
    /*static local_ptr create(const address_type& addr) {
        local_ptr result;
        result.addr_ = addr;
        return result;
    }*/
    
    void advance(const mgbase::ptrdiff_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::untyped::advanced(
            addr_
        ,   index * static_cast<mgbase::ptrdiff_t>(mgbase::runtime_size_of<T>())
        );
    }
    
    bool is_null() const MGBASE_NOEXCEPT {
        return mgcom::rma::to_integer(addr_) == 0;
    }
    
    address_type addr_;
};

template <typename T>
inline mgbase::uint64_t to_integer(const local_ptr<T>& ptr) MGBASE_NOEXCEPT {
    return mgcom::rma::to_integer(ptr.to_address());
}

template <typename T>
inline T* to_raw_pointer(const local_ptr<T>& ptr) MGBASE_NOEXCEPT {
    return static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(ptr.to_address()));
}

} // namespace rma
} // namespace mgcom



