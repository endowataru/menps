
#pragma once

#include <mgcom/rma/local_ptr.hpp>

namespace mgcom {
namespace rma {

template <typename T>
struct paired_local_ptr
{
    mgcom::process_id_t         proc;
    mgcom::rma::local_ptr<T>    ptr;
};

} // namespace rma
} // namespace mgcom

