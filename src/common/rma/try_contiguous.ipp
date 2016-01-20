
#pragma once

#include "try_contiguous.hpp"
#include "try_handlers.ipp"

namespace mgcom {
namespace rma {
namespace untyped {

namespace detail {

namespace /*unnamed*/ {

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

} // unnamed namespace

} // namespace detail

} // namespace untyped
} // namespace rma
} // namespace mgcom

