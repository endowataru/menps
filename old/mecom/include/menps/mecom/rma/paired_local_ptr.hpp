
#pragma once

#include <menps/mecom/rma/local_ptr.hpp>

namespace menps {
namespace mecom {
namespace rma {

template <typename T>
struct paired_local_ptr
{
    mecom::process_id_t         proc;
    mecom::rma::local_ptr<T>    ptr;
};

} // namespace rma
} // namespace mecom
} // namespace menps

