
#pragma once

#include "sender.hpp"
#include "receiver.hpp"
#include "am.hpp"
#include "common/mpi_base.hpp"
#include "common/mpi_error.hpp"
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace am {
namespace sender {

namespace /*unnamed*/ {

MGBASE_STATIC_ASSERT(sizeof(MPI_Request) < MGCOM_AM_HANDLE_SIZE, "MPI_Request must be smaller than MGCOM_AM_HANDLE_SIZE");

class impl {
public:
    void initialize()
    {
        tickets_ = new index_t[number_of_processes()];
        std::fill(tickets_, tickets_ + number_of_processes(), receiver::constants::max_num_tickets);
    }
    
    void finalize()
    {
        delete[] tickets_;
    }
    
    bool try_send(
        const am_message_buffer&    msg
    ,   process_id_t                dest_proc
    ,   MPI_Request*                request
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
        
        const int size = static_cast<int>(sizeof(am_message_buffer));
        
        const void* const buffer = &msg;
        
        mpi_error::check(
            MPI_Irsend(
                const_cast<void*>(buffer)   // buffer (const_cast is required for old MPI implementations)
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
    
    template <impl& self>
    class send_handlers
    {
        typedef send_handlers           handlers_type;
        typedef send_cb                 cb_type;
        typedef mgbase::deferred<void>  result_type;
        typedef result_type (func_type)(cb_type&);
        
    public:
        static result_type start(cb_type& cb) {
            MPI_Request* const request = reinterpret_cast<MPI_Request*>(cb.handle);
            
            if (self.try_send(cb.msg, cb.dest_proc, request))
                return test(cb);
            else
                return mgbase::make_deferred<func_type, &handlers_type::start>(cb);
        }
        
    private:
        static result_type test(cb_type& cb) {
            MPI_Request* const request = reinterpret_cast<MPI_Request*>(cb.handle);
            
            int flag;
            MPI_Status status;
            mpi_error::check(
                MPI_Test(request, &flag, &status)
            );
            
            if (flag)
                return mgbase::make_ready_deferred();
            else
                return mgbase::make_deferred<func_type, &handlers_type::test>(cb);
        }
    };
    
    
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


} // namespace unnamed

} // namespace sender
} // namespace am
} // namespace mgcom

