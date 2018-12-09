
#pragma once

#include <menps/mecom/rma/address.hpp>

#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/pointer_facade.hpp>

namespace menps {
namespace mecom {
namespace rma {

template <typename T>
class remote_ptr
    : public mefdn::pointer_facade<rma::remote_ptr, T>
{
    typedef mefdn::pointer_facade<rma::remote_ptr, T>  base;
    
public:
    typedef untyped::remote_address     address_type;
    
    #if 0
#ifdef MEFDN_CXX11_SUPPORTED
    remote_ptr() noexcept = default;
    
    template <typename U>
    /*implicit*/ remote_ptr(const remote_ptr<U>&) noexcept;
#endif
    #endif
    
    address_type to_address() const noexcept {
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
#ifdef MEFDN_CXX11_SUPPORTED
    explicit remote_ptr(address_type addr)
        : addr_(addr) { }
#endif
    #endif
    
    friend class mefdn::pointer_core_access;
    
    /*static remote_ptr create(const address_type& addr) {
        remote_ptr result;
        result.addr_ = addr;
        return result;
    }*/
    
    template <typename U>
    remote_ptr<U> cast_to() const noexcept {
        return remote_ptr<U>::cast_from(to_address());
    }
    
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
inline mefdn::uint64_t to_integer(const remote_ptr<T>& ptr) noexcept {
    return mecom::rma::to_integer(ptr.to_address());
}

} // namespace rma
} // namespace mecom
} // namespace menps

