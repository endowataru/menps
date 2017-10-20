
#pragma once

#include "paired_local_ptr.hpp"
#include "paired_remote_ptr.hpp"
#include "registration.hpp"

namespace menps {
namespace mecom {
namespace rma {

struct rma_policy
{
    template <typename T>
    static paired_remote_ptr<T> use_remote_ptr(const paired_local_ptr<T>& plptr)
    {
        return { plptr.proc, mecom::rma::use_remote_ptr(plptr.proc, plptr.ptr) };
    }
    
    static paired_remote_ptr<void> next_in_bytes(const paired_remote_ptr<void>& prptr, const mefdn::ptrdiff_t diff) noexcept
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
} // namespace mecom
} // namespace menps

