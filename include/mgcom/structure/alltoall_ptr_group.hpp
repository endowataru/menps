
#pragma once

#include <mgcom/collective.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace structure {

template <typename T>
class alltoall_ptr_group
{
public:
    explicit alltoall_ptr_group(T* const ptr)
        : ptrs_(new T*[mgcom::number_of_processes()])
    {
        mgcom::collective::allgather(&ptr, &ptrs_[0], 1);
    }
    
    alltoall_ptr_group(const alltoall_ptr_group&) = delete;
    alltoall_ptr_group& operator = (const alltoall_ptr_group&) = delete;
    
    #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
    alltoall_ptr_group(alltoall_ptr_group&&) MGBASE_NOEXCEPT_DEFAULT = default;
    #else
    alltoall_ptr_group(alltoall_ptr_group&& other) MGBASE_NOEXCEPT
        : ptrs_(mgbase::move(other.ptrs_))
    { }
    #endif
    
    #ifdef MGBASE_CXX11_MOVE_ASSIGNMENT_DEFAULT_SUPPORTED
    alltoall_ptr_group& operator = (alltoall_ptr_group&&) MGBASE_NOEXCEPT_DEFAULT = default;
    #else
    alltoall_ptr_group& operator = (alltoall_ptr_group&& other) MGBASE_NOEXCEPT
    {
        this->ptrs_ = mgbase::move(other.ptrs_);
        return *this;
    }
    #endif
    
    T* at_process(const process_id_t proc) const MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(valid_process_id(proc));
        
        return ptrs_[proc];
    }
    
private:
    mgbase::unique_ptr<T* []> ptrs_;
};

} // namespace structure
} // namespace mgcom

