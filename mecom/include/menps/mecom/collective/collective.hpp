
#pragma once

#include "requester.hpp"

namespace menps {
namespace mecom {
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
    requester&          req
,   const process_id_t  root
,   void* const         ptr
,   const index_t       num_bytes
) {
    const broadcast_params params = { root, ptr, num_bytes };
    req.broadcast(params);
}

inline void allgather(
    requester&          req
,   const void* const   src
,   void* const         dest
,   const index_t       num_bytes
) {
    const allgather_params params = { src, dest, num_bytes };
    req.allgather(params);
}

inline void alltoall(
    requester&          req
,   const void* const   src
,   void* const         dest
,   const index_t       num_bytes
) {
    const alltoall_params params = { src, dest, num_bytes };
    req.alltoall(params);
}

} // namespace untyped


template <typename T>
inline void broadcast(
    requester&          req
,   const process_id_t  root
,   T* const            ptr
,   const index_t       num_elems
) {
    untyped::broadcast(req, root, ptr, sizeof(T) * num_elems);
}

template <typename T>
inline void allgather(
    requester&          req
,   const T* const      src
,   T* const            dest
,   const index_t       num_elems
) {
    untyped::allgather(req, src, dest, sizeof(T) * num_elems);
}

template <typename T>
inline void alltoall(
    requester&      req
,   const T* const  src
,   T* const        dest
,   const index_t   num_elems
) {
    untyped::alltoall(req, src, dest, sizeof(T) * num_elems);
}

template <typename T>
inline void broadcast(
    const process_id_t  root
,   T* const            ptr
,   const index_t       num_elems
) {
    broadcast(requester::get_instance(), root, ptr, num_elems);
}

template <typename T>
inline void allgather(
    const T* const      src
,   T* const            dest
,   const index_t       num_elems
) {
    allgather(requester::get_instance(), src, dest, num_elems);
}

template <typename T>
inline void alltoall(
    const T* const  src
,   T* const        dest
,   const index_t   num_elems
) {
    alltoall(requester::get_instance(), src, dest, num_elems);
}

} // namespace collective
} // namespace mecom
} // namespace menps

