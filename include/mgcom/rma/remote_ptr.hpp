
#pragma once

#include <mgcom/rma/address.hpp>
#include <mgcom/rma/pointer.h>

#include <mgbase/type_traits.hpp>
#include <mgbase/pointer_facade.hpp>
#include <mgbase/runtime_sized.hpp>

namespace mgcom {
namespace rma {

template <typename T>
class remote_ptr
    : public mgbase::pointer_facade<rma::remote_ptr, T>
{
    typedef mgbase::pointer_facade<rma::remote_ptr, T>  base;
    
public:
    typedef untyped::remote_address     address_type;
    
    #if 0
#ifdef MGBASE_CXX11_SUPPORTED
    remote_ptr() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ remote_ptr(const remote_ptr<U>&) MGBASE_NOEXCEPT;
#endif
    #endif
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
    static remote_ptr cast_from(const address_type& addr) {
        remote_ptr result;
        result.addr_ = addr;
        return result;
    }
    
    operator remote_ptr<const T>() const {
        return this->template cast_to<const T>();
    }
    
private:
    #if 0
#ifdef MGBASE_CXX11_SUPPORTED
    explicit remote_ptr(address_type addr)
        : addr_(addr) { }
#endif
    #endif
    
    friend class mgbase::pointer_core_access;
    
    /*static remote_ptr create(const address_type& addr) {
        remote_ptr result;
        result.addr_ = addr;
        return result;
    }*/
    
    template <typename U>
    remote_ptr<U> cast_to() const MGBASE_NOEXCEPT {
        return remote_ptr<U>::cast_from(to_address());
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
inline mgbase::uint64_t to_integer(const remote_ptr<T>& ptr) MGBASE_NOEXCEPT {
    return mgcom::rma::to_integer(ptr.to_address());
}

} // namespace rma
} // namespace mgcom

