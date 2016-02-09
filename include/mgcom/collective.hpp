
#pragma once

#include <mgcom/common.hpp>
#include "collective.h"

namespace mgcom {
namespace collective {

void barrier();

namespace untyped {

void broadcast(
    process_id_t    root
,   void*           ptr
,   index_t         number_of_bytes
);

void allgather(
    const void*     src
,   void*           dest
,   index_t         number_of_bytes
);

void alltoall(
    const void*     src
,   void*           dest
,   index_t         number_of_bytes
);

} // namespace untyped

namespace /*unnamed*/ {

template <typename T>
inline void broadcast(
    const process_id_t  root
,   T* const            ptr
,   const index_t       number_of_elements
) {
    untyped::broadcast(root, ptr, sizeof(T) * number_of_elements);
}

template <typename T>
inline void allgather(
    const T* const      src
,   T* const            dest
,   const index_t       number_of_elements
) {
    untyped::allgather(src, dest, sizeof(T) * number_of_elements);
}

template <typename T>
inline void alltoall(
    const T* const  src
,   T* const        dest
,   const index_t   number_of_elements
) {
    untyped::alltoall(src, dest, sizeof(T) * number_of_elements);
}

} // unnamed namespace

} // namespace collective
} // namespace mgcom

