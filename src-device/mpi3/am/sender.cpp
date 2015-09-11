
#include "sender.hpp"
#include "receiver.hpp"
#include "../am.hpp"
#include "mpi_base.hpp"
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {

namespace am {
namespace sender {

namespace {

MGBASE_STATIC_ASSERT(sizeof(MPI_Request) < MGCOM_AM_HANDLE_SIZE, "MPI_Request must be smaller than MGCOM_AM_HANDLE_SIZE");

class impl {
public:
    void initialize() {
        tickets_ = new index_t[number_of_processes()];
        std::fill(tickets_, tickets_ + number_of_processes(), receiver::constants::max_num_tickets);
    }
    
    void finalize() {
        delete[] tickets_;
    }
    
    bool try_send(
        message&        msg
    ,   process_id_t    dest_proc
    ,   MPI_Request*    request
    ) {
        if (!get_ticket_to(dest_proc)) {
            MGBASE_LOG_DEBUG("msg:Failed to get a ticket.");
            return false;
        }
        
        if (!mpi_base::get_lock().try_lock()) {
            add_ticket(dest_proc, 1);
            return false;
        }
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        const int size = static_cast<int>(sizeof(message));
        
        mpi_error::check(
            MPI_Irsend(
                &msg                        // buffer
            ,   size                        // count
            ,   MPI_BYTE                    // datatype
            ,   static_cast<int>(dest_proc) // dest
            ,   get_tag()                   // tag
            ,   get_comm()                  // comm
            ,   request
            )
        );
        
        MGBASE_LOG_DEBUG("msg:MPI_Irsend succeeded.\tdest:{}\tsize:{}", dest_proc, size);
        
        return true;
    }
    
    void add_ticket(process_id_t dest_proc, index_t ticket) {
        MGBASE_LOG_DEBUG("msg:Added ticket.\tdest:{}\tbefore:{}\tdiff:{}", dest_proc, tickets_[dest_proc], ticket);
        tickets_[dest_proc] += ticket;
    }
    
private:
    bool get_ticket_to(process_id_t dest_proc)
    {
        // FIXME: atomic operations
        
        index_t& ticket = tickets_[dest_proc];
        if (ticket <= 0)
            return false;
        
        --ticket;
        return true;
    }
    
    index_t* tickets_;
};

impl g_impl;

void test_send(send_cb& cb) {
    MPI_Request* const request = reinterpret_cast<MPI_Request*>(cb.handle);
    
    int flag;
    MPI_Status status;
    mpi_error::check(
        MPI_Test(request, &flag, &status)
    );
    
    if (flag) {
        mgbase::control::finished(cb);
    }
}

void start_send(send_cb& cb) {
    MPI_Request* const request = reinterpret_cast<MPI_Request*>(cb.handle);
    
    if (g_impl.try_send(cb.msg, cb.dest_proc, request)) {
        mgbase::control::enter<send_cb, test_send>(cb);
    }
}

}

void initialize() {
    g_impl.initialize();
}
void finalize() {
    g_impl.finalize();
}

void add_ticket_to(process_id_t dest_proc, index_t ticket) {
    g_impl.add_ticket(dest_proc, ticket);
}

}

namespace detail {

void send_nb(send_cb& cb)
{
    cb.msg.ticket = receiver::pull_tickets_from(cb.dest_proc);
    mgbase::control::enter<send_cb, sender::start_send>(cb);
}

}

}

}

