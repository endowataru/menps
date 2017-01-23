
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

