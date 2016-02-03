
#pragma once

// DEPRECATED

#include <mgcom/rma/address.hpp>
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
    
    static bool try_(cb_type& cb, const mgbase::operation& on_complete)
    {
        return mgcom::rma::try_remote_read_async(
            cb.proc
        ,   remote_pointer<const mgbase::uint8_t>::cast_from(cb.remote_addr)
        ,   local_pointer<mgbase::uint8_t>::cast_from(cb.local_addr)
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
    
    static bool try_(cb_type& cb, const mgbase::operation& on_complete)
    {
        return mgcom::rma::try_remote_write_async(
            cb.proc
        ,   remote_pointer<mgbase::uint8_t>::cast_from(cb.remote_addr)
        ,   local_pointer<const mgbase::uint8_t>::cast_from(cb.local_addr)
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

