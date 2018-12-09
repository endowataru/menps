
#pragma once

#include <menps/mecom/rma/remote_ptr.hpp>

namespace menps {
namespace mecom {
namespace rma {

template <typename T>
struct paired_remote_ptr
{
    mecom::process_id_t         proc;
    mecom::rma::remote_ptr<T>   ptr;
};

} // namespace rma
} // namespace mecom
} // namespace menps

