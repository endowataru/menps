
#pragma once

#include <mgcom/rma/address.hpp>
#include <mgcom/rma/pointer.h>

#include <mgbase/type_traits.hpp>
#include <mgbase/pointer_facade.hpp>
#include <mgbase/runtime_sized.hpp>

namespace mgcom {
namespace rma {

template <typename T>
class remote_pointer
    : public mgbase::pointer_facade<rma::remote_pointer, T>
{
    typedef mgbase::pointer_facade<rma::remote_pointer, T>  base;
    
public:
    typedef untyped::remote_address     address_type;
    
#ifdef MGBASE_CPP11_SUPPORTED
    remote_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ remote_pointer(const remote_pointer<U>&) MGBASE_NOEXCEPT;
#endif
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
    static remote_pointer cast_from(const address_type& addr) {
        remote_pointer result;
        result.addr_ = addr;
        return result;
    }
    
private:
#ifdef MGBASE_CPP11_SUPPORTED
    explicit remote_pointer(address_type addr)
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
class local_pointer
    : public mgbase::pointer_facade<rma::local_pointer, T>
{
    typedef mgbase::pointer_facade<rma::local_pointer, T>   base;

public:
    typedef untyped::local_address      address_type;
    
#ifdef MGBASE_CPP11_SUPPORTED
    local_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ local_pointer(const local_pointer<U>&) MGBASE_NOEXCEPT;
#endif
    operator T* () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    T* operator -> () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    T& operator * () const MGBASE_NOEXCEPT {
        return *raw();
    }
    
    T* raw() const MGBASE_NOEXCEPT { return static_cast<T*>(untyped::to_raw_pointer(addr_)); }
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
    static local_pointer cast_from(const address_type& addr) {
        local_pointer result;
        result.addr_ = addr;
        return result;
    }

private:
#ifdef MGBASE_CPP11_SUPPORTED
    explicit local_pointer(address_type addr)
        : addr_(addr) { }
#endif
    
    friend class mgbase::pointer_core_access;
    
    template <typename U>
    local_pointer<U> cast_to() const MGBASE_NOEXCEPT {
        return local_pointer<U>::cast_from(to_address());
    }
    
    /*static local_pointer create(const address_type& addr) {
        local_pointer result;
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

namespace /*unnamed*/ {

template <typename T>
inline remote_pointer<T> use_remote_pointer(process_id_t proc_id, const local_pointer<T>& ptr) {
    return remote_pointer<T>::cast_from(mgcom::rma::untyped::use_remote_address(proc_id, ptr.to_address()));
}

template <typename T>
inline local_pointer<T> allocate(std::size_t number_of_elements = 1) {
    untyped::registered_buffer buf = untyped::allocate(mgbase::runtime_size_of<T>() * number_of_elements);
    return local_pointer<T>::cast_from(untyped::to_address(buf));
}

template <typename T>
inline void deallocate(const local_pointer<T>& ptr) {
    untyped::registered_buffer buf = { ptr.to_address() }; // TODO
    untyped::deallocate(buf);
}


template <typename T>
inline mgbase::uint64_t to_integer(const local_pointer<T>& ptr) MGBASE_NOEXCEPT {
    return mgcom::rma::to_integer(ptr.to_address());
}
template <typename T>
inline mgbase::uint64_t to_integer(const remote_pointer<T>& ptr) MGBASE_NOEXCEPT {
    return mgcom::rma::to_integer(ptr.to_address());
}

template <typename T>
inline T* to_raw_pointer(const local_pointer<T>& ptr) MGBASE_NOEXCEPT {
    return static_cast<T*>(mgcom::rma::untyped::to_raw_pointer(ptr.to_address()));
}

} // unnamed namespace

} // namespace rma
} // namespace mgcom

