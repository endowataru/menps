
#pragma once

#include <mgcom/rma/remote_ptr.hpp>

namespace mgcom {
namespace rma {

template <typename T>
struct paired_remote_ptr
{
    mgcom::process_id_t         proc;
    mgcom::rma::remote_ptr<T>   ptr;
};

} // namespace rma
} // namespace mgcom

