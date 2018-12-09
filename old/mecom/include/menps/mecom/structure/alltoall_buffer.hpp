
#pragma once

#include <menps/mecom/rma.hpp>
#include <menps/mecom/collective.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace structure {

template <typename T>
class alltoall_buffer
{
private:
    typedef mecom::rma::local_ptr<T>    local_ptr_type;
    typedef mecom::rma::remote_ptr<T>   remote_ptr_type;
    
public:
    void collective_initialize(const local_ptr_type& local_ptr)
    {
        const auto local_ptrs =
            mefdn::make_unique<local_ptr_type []>(number_of_processes());
        
        mecom::collective::allgather(&local_ptr, &local_ptrs[0], 1);
        
        remote_ptrs_ = mefdn::make_unique<remote_ptr_type []>(number_of_processes());
        
        for (process_id_t proc = 0; proc < number_of_processes(); ++proc)
            remote_ptrs_[proc] = mecom::rma::use_remote_ptr(proc, local_ptrs[proc]);
    }
    
    void finalize()
    {
        remote_ptrs_.reset();
    }
    
    remote_ptr_type at_process(process_id_t proc_id) const noexcept
    {
        MEFDN_ASSERT(valid_process_id(proc_id));
        
        return remote_ptrs_[proc_id];
    }

private:
    mefdn::unique_ptr<remote_ptr_type []> remote_ptrs_;
};

} // namespace structure
} // namespace mecom
} // namespace menps

