
#pragma once

#include "mgcom.hpp"
#include <mgbase/type_traits.hpp>

namespace mgcom {

namespace typed_rma {

typedef std::ptrdiff_t  index_diff_t;

namespace detail {

template <template <typename> class Derived, typename T, bool IsCompound = mgbase::is_compound<T>::value>
class pointer_base_member { };

template <template <typename> class Derived, typename T>
class pointer_base_member<Derived, T, true>
{
public:
    template <typename U>
    Derived<U> member(U (T::*q)) const {
        const index_t offset =
            reinterpret_cast<mgbase::uint8_t*>(&(static_cast<T*>(MGBASE_NULLPTR)->*q))
            - static_cast<mgbase::uint8_t*>(MGBASE_NULLPTR);
        
        return Derived<U>(rma::advanced(derived().to_address(), offset));
    }
    
private:
          Derived<T>& derived()       { return static_cast<      Derived<T>&>(*this); }
    const Derived<T>& derived() const { return static_cast<const Derived<T>&>(*this); }
};

template <template <typename> class Derived, typename T>
class pointer_base
    : public pointer_base_member<Derived, T>
{
    typedef Derived<T>  derived_type;
    
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


template <typename T, template <typename> class Derived, typename U>
inline Derived<T> static_pointer_cast(const pointer_base<Derived, U>&);


}

template <typename T>
class remote_pointer
    : public detail::pointer_base<typed_rma::remote_pointer, T>
    //: public detail::remote_pointer_base<T>
{
    typedef detail::pointer_base<typed_rma::remote_pointer, T>  base;
    
public:
    remote_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ remote_pointer(const remote_pointer<U>&);
    
    remote_pointer& operator += (index_t index) {
        addr_ = mgcom::rma::advanced(addr_, index * sizeof(T));
        return *this;
    }
    
    operator rma::remote_address() const MGBASE_NOEXCEPT {
        return to_address();
    }
    
    rma::remote_address to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
private:
    explicit remote_pointer(rma::remote_address addr)
        : addr_(addr) { }
    
    rma::remote_address addr_;
    
    template <template <typename> class Derived, typename U, bool IsCompound>
    friend class detail::pointer_base_member;
    
    template <template <typename> class Derived, typename U>
    friend class detail::pointer_base;
};

template <typename T>
class local_pointer
    : public detail::pointer_base<typed_rma::local_pointer, T>
{
    typedef detail::pointer_base<typed_rma::local_pointer, T>   base;
    
private:
    
    template <template <typename> class Derived, typename U, bool IsCompound>
    friend class detail::pointer_base_member;
    
    template <template <typename> class Derived, typename U>
    friend class detail::pointer_base;
};

namespace {

template <typename T>
inline void remote_read_nb(
    rma::remote_read_cb&            cb
,   process_id_t                    proc
,   const remote_pointer<const T>&  remote_ptr
,   const local_pointer<T>&         local_ptr
,   index_t                         number_of_elements
);

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

