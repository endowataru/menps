
#include <mgcom.hpp>
#include <mpi.h>
#include "mpi_error.hpp"
#include "am_message.hpp"

namespace mgcom {

namespace am {
namespace receiver {

namespace {

class impl
{
public:
    static const index_t max_num_callbacks = 1024;
    static const index_t max_num_requests = 32;
    
    void initialize()
    {
        callbacks_ = new am_handler_callback_t[max_num_callbacks];
        
        for (index_t i = 0; i < max_num_requests; ++i) {
            irecv(i);
        }
    }
    
    void finalize()
    {
        delete[] callbacks_;
    }
    
    void register_handler(am_handler_id_t id, am_handler_callback_t callback) {
        callbacks_[id] = callback;
    }
    
    void poll_am()
    {
        int index;
        int flag;
        MPI_Status status;
        
        /*
         * TODO : Thread safety of MPI_Tesyany.
         *        It's confirmed to be guaranteed on OpenMPI.
         */
        
        mpi_error::check(
            MPI_Testany(static_cast<int>(max_num_requests), requests_, &index, &flag, &status)
        );
        
        if (index == MPI_UNDEFINED)
            return;
        
        const process_id_t src = static_cast<process_id_t>(status.MPI_SOURCE);
        
        call(src, buffers_[index]);
        
        irecv(index);
    }
    
private:
    void call(process_id_t src, am_message& msg) {
        am_callback_parameter param;
        param.source = src;
        param.data   = msg.data;
        param.size   = msg.size;
        
        callbacks_[msg.id](&param);
    }
    
    void irecv(int index) {
        mpi_error::check(
            MPI_Irecv(&buffers_[index], sizeof(am_message), MPI_BYTE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &requests_[index])
        );
    }
    
    am_handler_callback_t* callbacks_;
    MPI_Request requests_[max_num_requests];
    
    am_message buffers_[max_num_requests];
};

impl g_impl;

}

void initialize()
{
    g_impl.initialize();
}

void finalize()
{
    g_impl.finalize();
}

}
}


void register_am_handler(
    am_handler_id_t       id
,   am_handler_callback_t callback
)
{
    am::receiver::g_impl.register_handler(id, callback);
}

void poll_am()
{
    am::receiver::g_impl.poll_am();
}


}

