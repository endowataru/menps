
#pragma once

#include <mgcom.hpp>
#include "collective.h"

namespace mgcom {
namespace collective {

typedef mgcom_collective_barrier_cb barrier_cb;

mgbase::deferred<void> barrier_nb(barrier_cb&);

namespace /*unnamed*/ {

inline void barrier()
{
    barrier_cb cb;
    barrier_nb(cb).wait();
}

} // unnamed namespace


typedef mgcom_collective_broadcast_cb   broadcast_cb;

namespace untyped {

mgbase::deferred<void> broadcast(broadcast_cb&);

} // namespace untyped

namespace /*unnamed*/ {

template <typename T>
inline mgbase::deferred<void> broadcast_nb(
    broadcast_cb&   cb
,   process_id_t    root
,   T*              ptr
,   index_t         number_of_elements
) {
    cb.root = root;
    cb.ptr  = ptr;
    cb.number_of_bytes = sizeof(T) * number_of_elements;
    
    return untyped::broadcast(cb);
}

template <typename T>
inline void broadcast(process_id_t root, T* ptr, index_t number_of_elements)
{
    broadcast_cb cb;
    broadcast_nb(cb, root, ptr, number_of_elements).wait();
}

} // unnamed namespace


typedef mgcom_collective_allgather_cb   allgather_cb;

namespace untyped {

mgbase::deferred<void> allgather(allgather_cb&);

} // namespace untyped

namespace /*unnamed*/ {

template <typename T>
inline mgbase::deferred<void> allgather_nb(
    allgather_cb&   cb
,   const T*        src
,   T*              dest
,   index_t         number_of_elements
) {
    cb.src  = src;
    cb.dest = dest;
    cb.number_of_bytes = sizeof(T) * number_of_elements;
    
    return untyped::allgather(cb);
}

template <typename T>
inline void allgather(const T* src, T* dest, index_t number_of_elements)
{
    allgather_cb cb;
    allgather_nb(cb, src, dest, number_of_elements).wait();
}

} // unnamed namespace

} // namespace collective
} // namespace mgcom

