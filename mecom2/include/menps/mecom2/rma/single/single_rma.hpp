
#pragma once

#include <menps/mecom2/rma/rma_typed_itf.hpp>
#include <menps/mecom2/rma/rma_pass_buf_copier.hpp>
#include <menps/mecom2/rma/rma_typed_allocator.hpp>
#include <menps/mecom2/rma/rma_private_heap_alloc.hpp>
#include <menps/mecom2/rma/basic_unique_public_ptr.hpp>
#include <menps/mefdn/memory/distance_in_bytes.hpp>

namespace menps {
namespace mecom2 {

class single_rma;

template <typename T>
struct single_unique_public_ptr_policy {
    using derived_type = basic_unique_public_ptr<single_unique_public_ptr_policy>;
    //using resource_type = T*;
    using resource_type = mefdn::remove_extent_t<T> *; // TODO
    
    using deleter_type = unique_public_ptr_deleter<single_unique_public_ptr_policy>;
    
    using allocator_type = single_rma;
};

template <typename T>
using single_unique_public_ptr = basic_unique_public_ptr<single_unique_public_ptr_policy<T>>;


struct single_rma_policy
{
    using derived_type = single_rma;
    
    using size_type = mefdn::size_t;
    using difference_type = mefdn::ptrdiff_t;
    
    using proc_id_type = size_type;
    
    template <typename T>
    using local_ptr = T*;
    template <typename T>
    using remote_ptr = T*;
    template <typename T>
    using public_ptr = T*;
    template <typename T>
    using unique_public_ptr = single_unique_public_ptr<T>;
    template <typename T>
    using unique_local_ptr = mefdn::unique_ptr<T>;
    
    template <typename Ptr>
    using element_type_of = mefdn::remove_pointer_t<Ptr>;
    
    template <typename U, typename T>
    static U* static_cast_to(T* const p) noexcept {
        return static_cast<U*>(p);
    }
    
};

class single_rma
    : public rma_pass_buf_copier<single_rma_policy>
    , public rma_typed_allocator<single_rma_policy>
    , public rma_private_heap_alloc<single_rma_policy>
{
    using policy_type = single_rma_policy;
    
public:
    using size_type = typename policy_type::size_type;
    using proc_id_type = typename policy_type::proc_id_type;
    
    template <typename T>
    using local_ptr = T*;
    template <typename T>
    using remote_ptr = T*;
    template <typename T>
    using public_ptr = T*;
    template <typename T>
    using unique_public_ptr = single_unique_public_ptr<T>;
    
    template <typename U, typename T>
    static U* member(T* const p, U (T::* const q)) noexcept {
        return &(p->*q);
    }
    
    template <typename T>
    T* attach(T* const first, T* const /*last*/) {
        // do nothing
        return first;
    }
    void detach(void* const /*first*/) {
        // do nothing
    }
    
    void progress() {
        // do nothing
    }
    
    void* untyped_allocate(const mefdn::size_t size) {
        return new mefdn::byte[size];
    }
    void untyped_deallocate(void* const p) {
        delete[] static_cast<mefdn::byte*>(p);
    }
    
    template <typename T>
    T compare_and_swap(
        const proc_id_type  target_proc
    ,   T* const            target_rptr
    ,   T                   expected
    ,   const T             desired
    ) {
        // TODO: breaking type safety
        auto& target = *reinterpret_cast<volatile std::atomic<T>*>(target_rptr);
        
        target.compare_exchange_strong(expected, desired, mefdn::memory_order_relaxed);
        
        return expected;
    }
    
    void write(
        const proc_id_type              /*dest_proc*/
    ,   const remote_ptr<void>&         dest_rptr
    ,   const local_ptr<const void>&    src_lptr
    ,   const size_type                 num_bytes
    ) {
        std::memcpy(dest_rptr, src_lptr, num_bytes);
    }
    
    void read(
        const proc_id_type              /*src_proc*/
    ,   const remote_ptr<const void>&   src_rptr
    ,   const local_ptr<void>&          dest_lptr
    ,   const size_type                 num_bytes
    ) {
        std::memcpy(dest_lptr, src_rptr, num_bytes);
    }
    
    size_type serialized_size_in_bytes(void* /*ptr*/) {
        return sizeof(void*);
    }
    void serialize(void* const ptr, void* const buf) {
        *reinterpret_cast<void**>(buf) = ptr;
    }
    template <typename T>
    T* deserialize(proc_id_type /*proc*/, const void* const buf) {
        return *reinterpret_cast<T* const *>(buf);
    }
};

using single_rma_ptr = mefdn::unique_ptr<single_rma>;

inline single_rma_ptr make_single_rma() {
    return mefdn::make_unique<single_rma>();
}

} // namespace mecom2
} // namespace menps

