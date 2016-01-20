
#pragma once

#include "device/mpi/mpi_base.hpp"
#include "am.hpp"

#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace am {

namespace /*unnamed*/ {

MGBASE_STATIC_ASSERT(sizeof(MPI_Request) < MGCOM_AM_HANDLE_SIZE, "MPI_Request must be smaller than MGCOM_AM_HANDLE_SIZE");

class sender_impl
    : public virtual resource_manager
{
    typedef resource_manager    base_manager;
    
public:
    template <typename T, T& Self>
    class send_handlers
    {
        typedef send_handlers           handlers_type;
        typedef send_cb                 cb_type;
        typedef mgbase::deferred<void>  result_type;
        typedef result_type (func_type)(cb_type&);
        
    public:
        static result_type start(cb_type& cb) {
            MPI_Request* const request = reinterpret_cast<MPI_Request*>(cb.handle);
            
            if (Self.try_send(cb.msg, cb.dest_proc, request))
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
    
private:
    bool try_send(
        am_message_buffer&  msg
    ,   process_id_t        dest_proc
    ,   MPI_Request*        request
    ) {
        if (!base_manager::try_use_remote_ticket_to(dest_proc)) {
            MGBASE_LOG_DEBUG("msg:Failed to get a ticket.");
            return false;
        }
        
        if (!mpi_base::get_lock().try_lock()) {
            base_manager::restore_remote_tickets_to(dest_proc, 1);
            return false;
        }
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        msg.ticket = base_manager::pull_local_tickets_from(dest_proc);
        
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
};


} // namespace unnamed

} // namespace am
} // namespace mgcom

