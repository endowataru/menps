
#pragma once

// DEPRECATED

#include <mgcom/rma/address.hpp>
#include "try_handlers.ipp"

namespace mgcom {
namespace rma {
namespace untyped {

namespace detail {

namespace /*unnamed*/ {

class remote_atomic_write_default_handlers
    : public try_handlers<remote_atomic_write_default_handlers, remote_atomic_write_default_cb>
{
public:
    typedef remote_atomic_write_default_cb  cb_type;
    
    static bool try_(cb_type& cb, const mgbase::operation& on_complete)
    {
        return try_remote_atomic_write_async(
            cb.proc
        ,   remote_pointer<atomic_default_t>::cast_from(cb.remote_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.local_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.buf_addr)
        ,   on_complete
        );
    }
};

class remote_atomic_read_default_handlers
    : public try_handlers<remote_atomic_read_default_handlers, remote_atomic_read_default_cb>
{
public:
    typedef remote_atomic_read_default_cb   cb_type;
    
    static bool try_(cb_type& cb, const mgbase::operation& on_complete)
    {
        return try_remote_atomic_read_async(
            cb.proc
        ,   remote_pointer<atomic_default_t>::cast_from(cb.remote_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.local_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.buf_addr)
        ,   on_complete
        );
    }
};

class remote_compare_and_swap_default_handlers
    : public try_handlers<remote_compare_and_swap_default_handlers, remote_compare_and_swap_default_cb>
{
public:
    typedef remote_compare_and_swap_default_cb  cb_type;
    
    static bool try_(cb_type& cb, const mgbase::operation& on_complete)
    {
        return try_remote_compare_and_swap_async(
            cb.target_proc
        ,   remote_pointer<atomic_default_t>::cast_from(cb.target_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.expected_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.desired_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.result_addr)
        ,   on_complete
        );
    }
    
    static void test(cb_type& /*cb*/) {
        poll();
    }
};

class remote_fetch_and_add_default_handlers
    : public try_handlers<remote_fetch_and_add_default_handlers, remote_fetch_and_add_default_cb>
{
public:
    typedef remote_fetch_and_add_default_cb     cb_type;
    
    static bool try_(cb_type& cb, const mgbase::operation& on_complete)
    {
        return try_remote_fetch_and_add_async(
            cb.target_proc
        ,   remote_pointer<atomic_default_t>::cast_from(cb.target_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.value_addr)
        ,   local_pointer<atomic_default_t>::cast_from(cb.result_addr)
        ,   on_complete
        );
    }
};

} // unnamed namespace

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom

