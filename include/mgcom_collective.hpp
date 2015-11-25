
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

namespace mgcom {

namespace collective {

template <typename T>
inline void broadcast(process_id_t root, T* ptr, index_t number_of_elements)
{
    // TODO: Polling of Active Messages
    
    MPI_Bcast(
        ptr
    ,   sizeof(T) * number_of_elements
    ,   MPI_BYTE
    ,   static_cast<int>(root)
    ,   MPI_COMM_WORLD // TODO
    );
}

template <typename T>
inline void allgather(const T* src, T* dest, index_t number_of_elements)
{
    // TODO: Polling of Active Messages
    
    MPI_Allgather(
        src
    ,   sizeof(T) * number_of_elements
    ,   MPI_BYTE
    ,   dest
    ,   sizeof(T) * number_of_elements
    ,   MPI_BYTE
    ,   MPI_COMM_WORLD // TODO
    );
}

inline void barrier() {
    mgcom::barrier();
}

}

}

