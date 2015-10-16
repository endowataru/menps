
#pragma once

#include <mgcom.hpp>
#include <mgcom_pointer.hpp>
#include <mgbase/scoped_ptr.hpp>

namespace mgcom {
namespace typed_rma {

template <typename T>
class alltoall_buffer {
public:
    void initialize() {
        
        
        mgbase::scoped_ptr<mgcom::typed_rma::local_pointer<T> []> local_ptrs(
            new mgcom::typed_rma::local_pointer<T>[number_of_processes()]
        );
        
        remote_ptrs_ = new mgcom::typed_rma::remote_pointer<T>[number_of_processes()];
        
        for (process_id_t proc = 0; proc < number_of_processes(); ++proc)
            remote_ptrs_[proc] = mgcom::typed_rma::use_remote_pointer(proc, local_ptrs[proc]);
    }
    
    void finalize() {
        remote_ptrs_.reset();
    }
    
    mgcom::typed_rma::remote_pointer<T> at_process(process_id_t proc_id) const MGBASE_NOEXCEPT {
        return remote_ptrs_[proc_id];
    }

private:
    mgcom::typed_rma::local_pointer<T> local_ptr;
    mgbase::scoped_ptr<mgcom::typed_rma::remote_pointer<T> []> remote_ptrs_;
};

}
}

