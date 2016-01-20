
#pragma once

#include "try_atomic.hpp"
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
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_remote_atomic_write_default(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.buf_addr
        ,   on_complete
        );
    }
};

class remote_atomic_read_default_handlers
    : public try_handlers<remote_atomic_read_default_handlers, remote_atomic_read_default_cb>
{
public:
    typedef remote_atomic_read_default_cb   cb_type;
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_remote_atomic_read_default(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.buf_addr
        ,   on_complete
        );
    }
};

class remote_compare_and_swap_default_handlers
    : public try_handlers<remote_compare_and_swap_default_handlers, remote_compare_and_swap_default_cb>
{
public:
    typedef remote_compare_and_swap_default_cb  cb_type;
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_remote_compare_and_swap_default(
            cb.target_proc
        ,   cb.target_addr
        ,   cb.expected_addr
        ,   cb.desired_addr
        ,   cb.result_addr
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
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_remote_fetch_and_add_default(
            cb.target_proc
        ,   cb.target_addr
        ,   cb.value_addr
        ,   cb.result_addr
        ,   on_complete
        );
    }
};

class local_compare_and_swap_default_handlers
    : public try_handlers<local_compare_and_swap_default_handlers, local_compare_and_swap_default_cb>
{
public:
    typedef local_compare_and_swap_default_cb  cb_type;
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_local_compare_and_swap_default(
            cb.target_addr
        ,   cb.expected_addr
        ,   cb.desired_addr
        ,   cb.result_addr
        ,   on_complete
        );
    }
};

class local_fetch_and_add_default_handlers
    : public try_handlers<local_fetch_and_add_default_handlers, local_fetch_and_add_default_cb>
{
public:
    typedef local_fetch_and_add_default_cb     cb_type;
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_local_fetch_and_add_default(
            cb.target_addr
        ,   cb.value_addr
        ,   cb.result_addr
        ,   on_complete
        );
    }
};

} // unnamed namespace

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom

