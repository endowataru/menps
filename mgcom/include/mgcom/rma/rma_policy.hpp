
#pragma once

#include "paired_local_ptr.hpp"
#include "paired_remote_ptr.hpp"
#include "registration.hpp"

namespace mgcom {
namespace rma {

struct rma_policy
{
    template <typename T>
    static paired_remote_ptr<T> use_remote_ptr(const paired_local_ptr<T>& plptr)
    {
        return { plptr.proc, mgcom::rma::use_remote_ptr(plptr.proc, plptr.ptr) };
    }
    
    static paired_remote_ptr<void> next_in_bytes(const paired_remote_ptr<void>& prptr, const mgbase::ptrdiff_t diff) MGBASE_NOEXCEPT
    {
        return {
            prptr.proc
        ,   remote_ptr<void>::cast_from(
                untyped::advanced(prptr.ptr.to_address(), diff)
            )
        };
    }
};

} // namespace rma
} // namespace mgcom

