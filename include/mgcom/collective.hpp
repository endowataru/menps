
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

namespace mgcom {
namespace collective {

namespace untyped {

void broadcast(process_id_t root, void* ptr, index_t number_of_bytes);

void allgather(const void* src, void* dest, index_t number_of_bytes);

} // namespace untyped

template <typename T>
inline void broadcast(process_id_t root, T* ptr, index_t number_of_elements)
{
    untyped::broadcast(
        root
    ,   ptr
    ,   sizeof(T) * number_of_elements
    );
}

template <typename T>
inline void allgather(const T* src, T* dest, index_t number_of_elements)
{
    // TODO: Polling of Active Messages
    untyped::allgather(
        src
    ,   dest
    ,   sizeof(T) * number_of_elements
    );
}

void barrier();

} // namespace collective
} // namespace mgcom

