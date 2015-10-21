
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

namespace mgcom {

namespace collective {

template <typename T>
inline void broadcast(process_id_t root, T* ptr, index_t number_of_elements)
{
    MPI_Bcast(
        ptr
    ,   sizeof(T) * number_of_elements
    ,   MPI_BYTE
    ,   static_cast<int>(root)
    ,   MPI_COMM_WORLD
    );
}

inline void barrier() {
    mgcom::barrier();
}

}

}

