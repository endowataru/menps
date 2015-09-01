
#include "sender.hpp"
#include "mpi_base.hpp"
#include <mgbase/threading/lock_guard.hpp>

namespace mgcom {

namespace am {
namespace sender {

namespace {

MGBASE_STATIC_ASSERT(sizeof(MPI_Request) < MGCOM_AM_HANDLE_SIZE, "MPI_Request must be smaller than MGCOM_AM_HANDLE_SIZE");

class impl {
public:
    void initialize()
    {
        
    }
    
    bool try_send(
        message&        msg
    ,   process_id_t    dest_proc
    ,   MPI_Request*    request
    ) {
        /*if (!get_resource_at(dest_proc))
            return false;*/
        
        if (!mpi_base::get_lock().try_lock()) {
            //release_resource_at(dest_proc);
            return false;
        }
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        
        const int size = static_cast<int>(sizeof(message));
        
        mpi_error::check(
            MPI_Isend(
                &msg                        // buffer
            ,   size                        // count
            ,   MPI_BYTE                    // datatype
            ,   static_cast<int>(dest_proc) // dest
            ,   0                           // tag  : TODO
            ,   MPI_COMM_WORLD              // comm : TODO
            ,   request
            )
        );
        
        return true;
    }
    
private:
    
};

impl g_impl;

void start_send(void* cb_) {
    send_cb& cb = *static_cast<send_cb*>(cb_);
    
    MPI_Request* request = reinterpret_cast<MPI_Request*>(cb.handle);
    
    if (g_impl.try_send(cb.msg, cb.dest_proc, request)) {
        
    }
}


}


}

void send(
    send_cb*     cb
,   handler_id_t id
,   const void*  value
,   index_t      size
,   process_id_t dest_proc
)
{
    cb->dest_proc = dest_proc;
    cb->msg.id    = id;
    cb->msg.size  = size;
    std::memcpy(cb->msg.data, value, size);
    
    mgbase::async_enter(cb, am::sender::start_send);
}

}

}

