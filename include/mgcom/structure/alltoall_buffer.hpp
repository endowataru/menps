
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/collective.hpp>
#include <mgbase/scoped_ptr.hpp>

namespace mgcom {
namespace structure {

template <typename T>
class alltoall_buffer
{
private:
    typedef mgcom::rma::local_pointer<T>    local_ptr_type;
    typedef mgcom::rma::remote_pointer<T>   remote_ptr_type;
    
public:
    void initialize()
    {
        mgbase::scoped_ptr<local_ptr_type []> local_ptrs(
            new local_ptr_type[number_of_processes()]
        );
        
        remote_ptrs_ = new remote_ptr_type[number_of_processes()];
        
        for (process_id_t proc = 0; proc < number_of_processes(); ++proc)
            remote_ptrs_[proc] = mgcom::rma::use_remote_pointer(proc, local_ptrs[proc]);
    }
    
    void finalize()
    {
        remote_ptrs_.reset();
    }
    
    remote_ptr_type at_process(process_id_t proc_id) const MGBASE_NOEXCEPT
    {
        return remote_ptrs_[proc_id];
    }

private:
    local_ptr_type local_ptr;
    mgbase::scoped_ptr<remote_ptr_type []> remote_ptrs_;
};

} // namespace structure
} // namespace mgcom

