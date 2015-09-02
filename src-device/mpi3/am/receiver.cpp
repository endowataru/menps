
#include <mgcom.hpp>
#include <mpi.h>
#include "mpi_error.hpp"
#include "mpi_base.hpp"
#include "../am.hpp"
#include "receiver.hpp"
#include "sender.hpp"

#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {

namespace am {
namespace receiver {

const index_t constants::max_num_tickets;

namespace {

class impl
{
public:
    static const index_t max_num_callbacks = 1024;
    
    void initialize()
    {
        callbacks_ = new handler_callback_t[max_num_callbacks];
        
        for (index_t i = 0; i < constants::max_num_tickets; ++i) {
            irecv(i);
        }
        
        // We need to issue a barrier to assure that
        // all processes have already issued MPI_Irecv()
        // before allowing the user to send messages
        MPI_Barrier(get_comm());
    }
    
    void finalize()
    {
        delete[] callbacks_;
    }
    
    void register_handler(handler_id_t id, handler_callback_t callback) {
        callbacks_[id] = callback;
    }
    
    void poll()
    {
        int index;
        int flag;
        MPI_Status status;
        
        if (!mpi_base::get_lock().try_lock())
            return;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        mpi_error::check(
            MPI_Testany(static_cast<int>(constants::max_num_tickets), requests_, &index, &flag, &status)
        );
        
        if (index == MPI_UNDEFINED)
            return;
        
        const process_id_t src = static_cast<process_id_t>(status.MPI_SOURCE);
        
        message& msg = buffers_[index];
        
        call(src, msg);
        
        irecv(index);
        
        sender::add_ticket(src, msg.ticket);
    }
    
private:
    void call(process_id_t src, message& msg) {
        callback_parameters params;
        params.source = src;
        params.data   = msg.data;
        params.size   = msg.size;
        
        MGBASE_LOG_DEBUG("Invoking callback: src={}, id={}", src, msg.id);
        
        callbacks_[msg.id](&params);
        
        MGBASE_LOG_DEBUG("Finished callback");
    }
    
    void irecv(int index) {
        mpi_error::check(
            MPI_Irecv(&buffers_[index], sizeof(message), MPI_BYTE, MPI_ANY_SOURCE, 0, get_comm(), &requests_[index])
        );
    }
    
    handler_callback_t* callbacks_;
    
    MPI_Request requests_[constants::max_num_tickets];
    message buffers_[constants::max_num_tickets];
};

impl g_impl;

}

void initialize() {
    g_impl.initialize();
}

void finalize() {
    g_impl.finalize();
}

void poll() {
    g_impl.poll();
}

}

void register_handler(
    handler_id_t       id
,   handler_callback_t callback
)
{
    receiver::g_impl.register_handler(id, callback);
}

}

}

