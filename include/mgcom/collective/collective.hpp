
#pragma once

#include "requester.hpp"

namespace mgcom {
namespace collective {

inline void barrier()
{
    requester::get_instance().barrier();
}

namespace untyped {

inline void broadcast(const broadcast_params& params)
{
    requester::get_instance().broadcast(params);
}

inline void allgather(const allgather_params& params)
{
    requester::get_instance().allgather(params);
}

inline void alltoall(const alltoall_params& params)
{
    requester::get_instance().alltoall(params);
}

} // namespace untyped


// For compatibility

namespace untyped {

inline void broadcast(
    const process_id_t  root
,   void* const         ptr
,   const index_t       num_bytes
) {
    const broadcast_params params = { root, ptr, num_bytes };
    broadcast(params);
}

inline void allgather(
    const void* const   src
,   void* const         dest
,   const index_t       num_bytes
) {
    const allgather_params params = { src, dest, num_bytes };
    allgather(params);
}

inline void alltoall(
    const void* const   src
,   void* const         dest
,   const index_t       num_bytes
) {
    const alltoall_params params = { src, dest, num_bytes };
    alltoall(params);
}

} // namespace untyped


template <typename T>
inline void broadcast(
    const process_id_t  root
,   T* const            ptr
,   const index_t       num_elems
) {
    untyped::broadcast(root, ptr, sizeof(T) * num_elems);
}

template <typename T>
inline void allgather(
    const T* const      src
,   T* const            dest
,   const index_t       num_elems
) {
    untyped::allgather(src, dest, sizeof(T) * num_elems);
}

template <typename T>
inline void alltoall(
    const T* const  src
,   T* const        dest
,   const index_t   num_elems
) {
    untyped::alltoall(src, dest, sizeof(T) * num_elems);
}

} // namespace collective
} // namespace mgcom

