
#pragma once

#include <mgcom.hpp>

#include "try_contiguous.hpp"
#include "common/rma/rma.hpp"
#include "common/notifier.hpp"

namespace mgcom {
namespace rma {
namespace untyped {

namespace detail {

namespace /*unnamed*/ {

template <typename Derived, typename CB>
class try_handlers
{
    typedef CB  cb_type;
    
public:
    static mgbase::deferred<void> start(cb_type& cb) {
        cb.finished = false;
        
        if (Derived::try_(cb, make_notifier_assign(&cb.finished, true))) {
            return test(cb);
        }
        else {
            mgcom::rma::poll(); // TODO
            
            return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), &try_handlers::start>(cb);
        }
    }
    
private:
    static mgbase::deferred<void> test(cb_type& cb)
    {
        if (cb.finished) {
            return mgbase::make_ready_deferred();
        }
        else {
            mgcom::rma::poll(); // TODO
            
            return mgbase::make_deferred<mgbase::deferred<void> (cb_type&), &try_handlers::test>(cb);
        }
    }
};

class remote_read_handlers
    : public try_handlers<remote_read_handlers, remote_read_cb>
{
public:
    typedef remote_read_cb      cb_type;
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_remote_read(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.size_in_bytes
        ,   on_complete
        );
    }
};

class remote_write_handlers
    : public try_handlers<remote_write_handlers, remote_write_cb>
{
public:
    typedef remote_write_cb     cb_type;
    
    static bool try_(cb_type& cb, const local_notifier& on_complete)
    {
        return try_remote_write(
            cb.proc
        ,   cb.remote_addr
        ,   cb.local_addr
        ,   cb.size_in_bytes
        ,   on_complete
        );
    }
};

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
        ,   cb.diff_addr
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

