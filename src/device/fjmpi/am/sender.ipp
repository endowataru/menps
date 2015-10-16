
#pragma once

#include "common/mpi_base.hpp"
#include "common/notifier.hpp"
#include <mgbase/threading/lock_guard.hpp>

#include "common/am/basic_sender.ipp"

#include <mgcom/alltoall_buffer.hpp>

#include "rma/rma.hpp"

#include "mpi-ext.h"

namespace mgcom {
namespace am {
namespace sender {

namespace {

class impl
    : public basic_impl
{
    typedef basic_impl  base;

public:
    void initialize()
    {
        buffers_.initialize();
    }
    
    void finalize()
    {
        buffers_.finalize();
    }
    
    bool try_send(
        message&                //msg
    ,   process_id_t            dest_proc
    ,   const local_notifier&   notifier
    ) {
        if (!base::get_ticket_to(dest_proc))
            return false;
        
        if (mpi_base::get_lock().try_lock()) {
            base::add_ticket(dest_proc, 1);
            return false;
        }
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        rma::registered_buffer buf = rma::allocate(sizeof(message));
        
        const typed_rma::remote_pointer<message> remote_ptr =
            buffers_.at_process(dest_proc).member(&message_buffer::msgs);
        
        mgcom::rma::try_remote_write_extra(
            dest_proc
        ,   remote_ptr.to_address()
        ,   rma::to_address(buf)
        ,   sizeof(message)
        ,   notifier
        ,   FJMPI_RDMA_REMOTE_NOTICE
        );
        
        //const int size = static_cast<int>(sizeof(message));
        
        return true;
    }
    
private:
    mgcom::typed_rma::alltoall_buffer<message_buffer> buffers_;
};

}

}
}
}

