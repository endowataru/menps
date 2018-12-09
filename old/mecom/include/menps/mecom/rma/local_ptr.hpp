
#pragma once

#include <menps/mecom/rma/address.hpp>

#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/pointer_facade.hpp>

namespace menps {
namespace mecom {
namespace rma {

template <typename T>
class local_ptr;

namespace detail {

template <
    typename T
,   bool Dereferencable = ! mefdn::is_void<T>::value
>
class local_ptr_deref { };

template <typename T>
class local_ptr_deref<T, true>
{
    typedef local_ptr<T>    derived_type;
    
public:
    T* operator -> () const noexcept {
        return derived().raw();
    }
    T& operator * () const noexcept {
        return *derived().raw();
    }
    
private:
    const derived_type& derived() const noexcept {
        return static_cast<const derived_type&>(*this);
    }
};

} // namespace detail

template <typename T>
class local_ptr
    : public mefdn::pointer_facade<rma::local_ptr, T>
    , public detail::local_ptr_deref<T>
{
    typedef mefdn::pointer_facade<rma::local_ptr, T>   base;

public:
    typedef untyped::local_address      address_type;
    
#ifdef MEFDN_CXX11_SUPPORTED
    local_ptr() noexcept = default;
    
    template <typename U>
    /*implicit*/ local_ptr(const local_ptr<U>&) noexcept;
#endif
    operator T* () const noexcept {
        return raw();
    }
    
    /*T* operator -> () const noexcept {
        return raw();
    }
    
    T& operator * () const noexcept {
        return *raw();
    }*/
    
    T* raw() const noexcept { return static_cast<T*>(untyped::to_raw_pointer(addr_)); }
    
    address_type to_address() const noexcept {
        return addr_;
    }
    
    static local_ptr cast_from(const address_type& addr) {
        local_ptr result;
        result.addr_ = addr;
        return result;
    }

private:
#ifdef MEFDN_CXX11_SUPPORTED
    explicit local_ptr(address_type addr)
        : addr_(addr) { }
#endif
    
    friend class mefdn::pointer_core_access;
    
    template <typename U>
    local_ptr<U> cast_to() const noexcept {
        return local_ptr<U>::cast_from(to_address());
    }
    
    /*static local_ptr create(const address_type& addr) {
        local_ptr result;
        result.addr_ = addr;
        return result;
    }*/
    
    void advance(const mefdn::ptrdiff_t index) noexcept {
        addr_ = mecom::rma::untyped::advanced(
            addr_
        ,   index * static_cast<mefdn::ptrdiff_t>(sizeof(T))
        );
    }
    
    bool is_null() const noexcept {
        return mecom::rma::to_integer(addr_) == 0;
    }
    
    address_type addr_;
};

template <typename T>
inline mefdn::uint64_t to_integer(const local_ptr<T>& ptr) noexcept {
    return mecom::rma::to_integer(ptr.to_address());
}

template <typename T>
inline T* to_raw_pointer(const local_ptr<T>& ptr) noexcept {
    return static_cast<T*>(mecom::rma::untyped::to_raw_pointer(ptr.to_address()));
}

} // namespace rma
} // namespace mecom
} // namespace menps



