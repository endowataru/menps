
#pragma once

#include <mgcom/rma/registrator.hpp>
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace rma {

namespace untyped {

/**
 * Register a region located on the current process.
 */
inline local_region register_region(
    void*   local_ptr
,   index_t size_in_bytes
) {
    const register_region_params params = { local_ptr, size_in_bytes };
    return registrator::get_instance().register_region(params);
}

/**
 * Prepare a region located on a remote process.
 */
inline remote_region use_remote_region(
    registrator&        reg
,   const process_id_t  proc_id
,   const region_key&   key
) {
    const use_remote_region_params params = { proc_id, key };
    return reg.use_remote_region(params);
}
inline remote_region use_remote_region(
    const process_id_t  proc_id
,   const region_key&   key
) {
    return use_remote_region(registrator::get_instance(), proc_id, key);
}

inline remote_address use_remote_address(registrator& reg, process_id_t proc_id, const local_address& addr) {
    remote_address result = { use_remote_region(reg, proc_id, addr.region.key), addr.offset };
    return result;
}
inline remote_address use_remote_address(process_id_t proc_id, const local_address& addr) {
    return use_remote_address(registrator::get_instance(), proc_id, addr);
}

/**
 * De-register the region located on the current process.
 */
inline void deregister_region(const local_region& region)
{
    const deregister_region_params params = { region };
    registrator::get_instance().deregister_region(params);
}


inline local_region allocate_region(index_t size_in_bytes)
{
    void* const ptr = new mgbase::uint8_t[size_in_bytes];
    return register_region(ptr, size_in_bytes);
}

inline void deallocate_region(const local_region& region)
{
    deregister_region(region);
    delete[] static_cast<mgbase::uint8_t*>(to_raw_pointer(region));
}

} // namespace untyped

template <typename T>
inline remote_ptr<T> use_remote_ptr(
    registrator&        reg
,   const process_id_t  proc_id
,   const local_ptr<T>& ptr
) {
    return remote_ptr<T>::cast_from(
        mgcom::rma::untyped::use_remote_address(reg, proc_id, ptr.to_address())
    );
}
template <typename T>
inline remote_ptr<T> use_remote_ptr(
    const process_id_t  proc_id
,   const local_ptr<T>& ptr
) {
    return use_remote_ptr(registrator::get_instance(), proc_id, ptr);
}

} // namespace rma
} // namespace mgcom

