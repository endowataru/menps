
#pragma once

#include <menps/mecom/collective.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace structure {

template <typename T>
class alltoall_ptr_group
{
public:
    explicit alltoall_ptr_group(T* const ptr)
        : ptrs_(new T*[mecom::number_of_processes()])
    {
        mecom::collective::allgather(&ptr, &ptrs_[0], 1);
    }
    
    alltoall_ptr_group(const alltoall_ptr_group&) = delete;
    alltoall_ptr_group& operator = (const alltoall_ptr_group&) = delete;
    
    MEFDN_DEFINE_DEFAULT_MOVE_NOEXCEPT_1(alltoall_ptr_group, ptrs_)
    
    T* at_process(const process_id_t proc) const noexcept
    {
        MEFDN_ASSERT(valid_process_id(proc));
        
        return ptrs_[proc];
    }
    
private:
    mefdn::unique_ptr<T* []> ptrs_;
};

} // namespace structure
} // namespace mecom
} // namespace menps

