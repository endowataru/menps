
#pragma once

#include <mgcom/rma/untyped.hpp>
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
    
#if MGBASE_CPP11_SUPPORTED
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
#if MGBASE_CPP11_SUPPORTED
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
    
    void advance(index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::untyped::advanced(addr_, index * mgbase::runtime_size_of<T>());
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
    
#if MGBASE_CPP11_SUPPORTED
    local_pointer() MGBASE_NOEXCEPT MGBASE_EMPTY_DEFINITION
    
    template <typename U>
    /*implicit*/ local_pointer(const local_pointer<U>&) MGBASE_NOEXCEPT;
#endif
    operator T* () const MGBASE_NOEXCEPT {
        return raw();
    }
    
    T* raw() const MGBASE_NOEXCEPT { return static_cast<T*>(untyped::to_pointer(addr_)); }
    
    address_type to_address() const MGBASE_NOEXCEPT {
        return addr_;
    }
    
    static local_pointer cast_from(const address_type& addr) {
        local_pointer result;
        result.addr_ = addr;
        return result;
    }

private:
#if MGBASE_CPP11_SUPPORTED
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
    
    void advance(index_t index) MGBASE_NOEXCEPT {
        addr_ = mgcom::rma::untyped::advanced(addr_, index * mgbase::runtime_size_of<T>());
    }
    
    bool is_null() const MGBASE_NOEXCEPT {
        return mgcom::rma::to_integer(addr_) == 0;
    }
    
    address_type addr_;
};

namespace /*unnamed*/ {

/// Simple remote read.
template <typename Remote, typename Local>
inline typename mgbase::enable_if<
    mgbase::is_runtime_sized_assignable<Local, Remote>::value
>::type
remote_read_nb(
    rma::remote_read_cb&            cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
) {
    mgcom::rma::untyped::remote_read_nb(
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
    mgbase::is_runtime_sized_assignable<Remote, Local>::value
>::type
remote_write_nb(
    rma::remote_write_cb&           cb
,   process_id_t                    proc
,   const remote_pointer<Remote>&   remote_ptr
,   const local_pointer<Local>&     local_ptr
) {
    mgcom::rma::untyped::remote_write_nb(
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
    mgcom::rma::untyped::remote_read_nb(
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
,   const local_pointer<const T>&   local_ptr
,   index_t                         number_of_elements
) {
    mgcom::rma::untyped::remote_write_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   number_of_elements * mgbase::runtime_size_of<T>()
    );
}

inline void remote_atomic_read_default_nb(
    remote_atomic_read_default_cb&                          cb
,   process_id_t                                            proc
,   const remote_pointer<const rma::atomic_default_t>&      remote_ptr
,   const local_pointer<rma::atomic_default_t>&             local_ptr
,   const local_pointer<rma::atomic_default_t>&             buf_ptr
) {
    mgcom::rma::untyped::remote_atomic_read_default_nb(
        cb
    ,   proc
    ,   remote_ptr.to_address()
    ,   local_ptr.to_address()
    ,   buf_ptr.to_address()
    );
}

inline void remote_atomic_write_default_nb(
    remote_atomic_write_default_cb&                         cb
,   process_id_t                                            proc
,   const remote_pointer<rma::atomic_default_t>&            remote_ptr
,   const local_pointer<const rma::atomic_default_t>&       local_ptr
,   const local_pointer<rma::atomic_default_t>&             buf_ptr
) {
    mgcom::rma::untyped::remote_atomic_write_default_nb(
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
    mgcom::rma::untyped::local_compare_and_swap_default_nb(
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
    mgcom::rma::untyped::remote_compare_and_swap_default_nb(
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
    mgcom::rma::untyped::remote_fetch_and_add_default_nb(
        cb
    ,   target_proc
    ,   target_ptr.to_address()
    ,   value_ptr.to_address()
    ,   result_ptr.to_address()
    );
}


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


} // unnamed namespace

} // namespace rma
} // namespace mgcom

