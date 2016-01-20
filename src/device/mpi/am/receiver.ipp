
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

#include "common/am/basic_receiver.hpp"
#include "device/mpi/mpi_base.hpp"
#include "am.hpp"
#include "resource_manager.ipp"

#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace am {

namespace /*unnamed*/ {

class receiver_impl
    : public basic_receiver
    , public virtual resource_manager
{
    typedef basic_receiver      base_receiver;
    typedef resource_manager    base_manager;
    
protected:
    receiver_impl() : initialized_(false) { }
    
    void initialize()
    {
        base_receiver::initialize(base_manager::max_num_tickets);
        
        requests_ = new MPI_Request[base_manager::max_num_tickets];
        
        for (index_t i = 0; i < base_manager::max_num_tickets; ++i) {
            irecv(i);
        }
        
        // We need to issue a barrier to assure that
        // all processes have already issued MPI_Irecv()
        // before allowing the user to send messages
        MPI_Barrier(get_comm());
        
        initialized_ = true;
    }
    
    void finalize()
    {
        for (index_t i = 0; i < base_manager::max_num_tickets; ++i) {
            MPI_Cancel(&requests_[i]);
        }
        
        requests_.reset();
        
        base_receiver::finalize();
    }
    
public:
    void poll()
    {
        MGBASE_ASSERT(initialized_);
        
        int index;
        int flag;
        MPI_Status status;
        
        if (!mpi_base::get_lock().try_lock())
            return;
        
        {
            mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
            
            mpi_error::check(
                MPI_Testany(static_cast<int>(base_manager::max_num_tickets), requests_.get(), &index, &flag, &status)
            );
            
            if (index == MPI_UNDEFINED)
                return;
        }
        
        const process_id_t src = static_cast<process_id_t>(status.MPI_SOURCE);
        
        am_message_buffer& msg = base_receiver::get_buffer_at(index);
        base_manager::restore_remote_tickets_to(src, msg.ticket);
        base_manager::push_local_ticket_from(src);
        
        base_receiver::call(src, msg);
        
        // TODO : Remove blocking
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        irecv(index);
    }
    
private:
    void irecv(index_t index)
    {
        am_message_buffer& buf = base_receiver::get_buffer_at(index);
        MPI_Request& request = requests_[index];
        
        mpi_error::check(
            MPI_Irecv(
                &buf                        // buf
            ,   sizeof(am_message_buffer)   // count
            ,   MPI_BYTE                    // datatype
            ,   MPI_ANY_SOURCE              // source
            ,   get_tag()                   // tag
            ,   get_comm()                  // comm
            ,   &request                    // request
            )
        );
    }
    
    bool initialized_;
    
    mgbase::scoped_ptr<MPI_Request []>  requests_;
};

} // unnamed namespace

} // namespace am
} // namespace mgcom

