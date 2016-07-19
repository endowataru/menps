
#pragma once

#include "rpc_resource_manager.hpp"
#include <mgcom/rma.hpp>
#include <mgcom/collective.hpp>
#include <algorithm>
#include "device/mpi/mpi_interface.hpp" // TODO: depends on MPI

namespace mgcom {
namespace rpc {

template <typename BufferT, typename TicketT, TicketT MaxConns, typename NicT, NicT NumNics>
class rpc_basic_connection_pool
{
    typedef BufferT     buffer_type;
    typedef TicketT     ticket_type;
    typedef NicT        nic_type;
    
    static const ticket_type max_num_connections = MaxConns;
    static const nic_type number_of_nics = NumNics;
    
    typedef mgcom::rma::local_ptr<buffer_type>   local_buffer_ptr_type;
    typedef mgcom::rma::remote_ptr<buffer_type>  remote_buffer_ptr_type;
    
public:
    void initialize(mpi::mpi_interface& mi, rma::allocator& alloc, rma::registrator& reg)
    {
        ep_ = &mi.get_endpoint();
        alloc_ = &alloc;
        
        manager_.initialize(*ep_, max_num_connections);
        
        info_by_procs_ = new processor_info[ep_->number_of_processes()];
        
        const mgbase::scoped_ptr<local_buffer_ptr_type []> local_receicer_bufs(
            new local_buffer_ptr_type[ep_->number_of_processes()]
        );
        
        for (process_id_t proc = 0; proc < ep_->number_of_processes(); ++proc)
        {
            // Allocate a sender buffer.
            
            info_by_procs_[proc].sender_local_buf
                = rma::allocate<buffer_type>(alloc, number_of_nics * max_num_connections);
            
            // Allocate a receiver buffer.
            
            const local_buffer_ptr_type receiver_buf
                = rma::allocate<buffer_type>(alloc, number_of_nics * max_num_connections);
            
            info_by_procs_[proc].receiver_local_buf = receiver_buf;
            local_receicer_bufs[proc] = receiver_buf;
        }
        
        const mgbase::scoped_ptr<local_buffer_ptr_type []> remote_receiver_bufs(
            new local_buffer_ptr_type[ep_->number_of_processes()]
        );
        
        // All-to-all address exchange.
        mpi::native_alltoall(mi, &local_receicer_bufs[0], &remote_receiver_bufs[0], 1);
        
        for (process_id_t proc = 0; proc < ep_->number_of_processes(); ++proc)
        {
            info_by_procs_[proc].sender_remote_buf
                = mgcom::rma::use_remote_ptr(reg, proc, remote_receiver_bufs[proc]);
        }
        
        MGBASE_LOG_DEBUG("msg:Initialized RPC connection pool.");
    }
    
    void finalize()
    {
        for (process_id_t proc = 0; proc < ep_->number_of_processes(); ++proc)
        {
            rma::deallocate(*alloc_, info_by_procs_[proc].sender_local_buf);
            rma::deallocate(*alloc_, info_by_procs_[proc].receiver_local_buf);
        }
        
        info_by_procs_.reset();
        
        manager_.finalize();
        
        MGBASE_LOG_DEBUG("msg:Finalized RPC connection pool.");
    }
    
    struct sender_info
    {
        nic_type                                    remote_nic;
        mgcom::rma::local_ptr<buffer_type>      local_buf;
        mgcom::rma::remote_ptr<buffer_type>     remote_buf;
    };
    
    bool try_start_sending(
        const process_id_t  receiver_proc
    ,   sender_info* const  result
    ) MGBASE_NOEXCEPT
    {
        nic_type remote_nic;
        ticket_type local_tickets[number_of_nics];
        
        // Try to use a ticket.
        if (!manager_.try_start_sending(receiver_proc, &remote_nic, local_tickets))
            return false;
        
        processor_info& info = info_by_procs_[receiver_proc];
        
        // Decide the buffer position.
        // This fetch_add order is not required to in order
        // because the resource is already acquired here.
        
        const ticket_type sender_count = info.sender_counts[remote_nic]++;
        
        const ticket_type sender_pos = get_buffer_position(remote_nic, sender_count);
        
        // Get the pointers corresponding to the position.
        
        const mgcom::rma::local_ptr<buffer_type> local_buf
            = info.sender_local_buf + sender_pos;
        
        const mgcom::rma::remote_ptr<buffer_type> remote_buf
            = info.sender_remote_buf + sender_pos;
        
        buffer_type* const buf = local_buf;
        
        // Copy the local tickets.
        for (nic_type nic = 0; nic < number_of_nics; ++nic)
            buf->tickets[nic] = local_tickets[nic];
        
        // Return the buffers.
        
        result->remote_nic = remote_nic;
        result->local_buf  = local_buf;
        result->remote_buf = remote_buf;
        
        return true;
    }
    
    void finish_sending(
        const process_id_t  receiver_proc
    ,   const nic_type      remote_nic
    ) MGBASE_NOEXCEPT
    {
        manager_.finish_sending(receiver_proc, remote_nic);
    }
    
    // This function is guaranteed to be called in the message arrival order.
    // However, because sender threads might try to use the local tickets concurrently and break the local buffers,
    // they must be carefully restored after a RPC call is confirmed to be finished.
    
    const buffer_type& start_receiving(
        const process_id_t  sender_proc
    ,   const nic_type      local_nic
    ,   ticket_type* const  receiver_count_result
    ) MGBASE_NOEXCEPT
    {
        processor_info& info = info_by_procs_[sender_proc];
        
        // Decide the buffer position.
        // Atomic operation is not needed because this function itself is locked by outside mutex.
        
        const ticket_type receiver_count = info.receiver_latest_counts[local_nic]++;
        
        const ticket_type receiver_pos = get_buffer_position(local_nic, receiver_count);
        
        // Get the pointer corresponding to the position.
        
        const mgcom::rma::local_ptr<buffer_type> local_buf
            = info.receiver_local_buf + receiver_pos;
        
        buffer_type& buf = *local_buf;
        
        // Restore the remote ticket.t.
        manager_.start_receiving(sender_proc, buf.tickets);
        
        // Although this value is not used by the caller function,
        // it is needed to synchronize the recycling of message buffers.
        *receiver_count_result = receiver_count;
        
        return *local_buf;
    }
    
    void finish_receiving(
        const process_id_t  sender_proc
    ,   const nic_type      local_nic
    ,   const ticket_type   receiver_count
    ) MGBASE_NOEXCEPT
    {
        processor_info& info = info_by_procs_[sender_proc];
        
        // Increment the count.
        // If other message handling threads are processing their messages,
        // receiver_processed_pos cannot be replaced by this thread.
        
        while (true)
        {
            ticket_type expected = receiver_count;
            
            if (info.receiver_processed_counts[local_nic].compare_exchange_weak(
                expected                // expected
            ,   receiver_count + 1      // desired
            ,   mgbase::memory_order_acquire
            ))
                break;
            
            // TODO: change to ordinary load instruction
        }
        
        // Restore a local ticket.
        manager_.finish_receiving(sender_proc, local_nic);
    }
    
private:
    static ticket_type get_buffer_position(const nic_type nic, const ticket_type pos)
    {
        const ticket_type index = pos % max_num_connections;
        
        const ticket_type result = nic * max_num_connections + index;
        MGBASE_ASSERT(0 <= result && result < number_of_nics * max_num_connections);
        return result;
    }
    
    struct processor_info
    {
        ticket_type                     sender_counts[number_of_nics];
        
        ticket_type                     receiver_latest_counts[number_of_nics];
        mgbase::atomic<ticket_type>     receiver_processed_counts[number_of_nics];
        
        mgcom::rma::local_ptr<buffer_type>       sender_local_buf;
        mgcom::rma::remote_ptr<buffer_type>      sender_remote_buf;
        
        mgcom::rma::local_ptr<buffer_type>       receiver_local_buf;
        
        processor_info()
        {
            for (nic_type nic = 0; nic < number_of_nics; ++nic)
            {
                sender_counts[nic] = 0;
                
                receiver_latest_counts[nic] = 0;
                receiver_processed_counts[nic].store(0, mgbase::memory_order_relaxed);
            }
        }
    };
    
    endpoint*                                       ep_;
    rma::allocator*                                 alloc_;
    rpc_resource_manager<TicketT, NicT, NumNics>    manager_;
    mgbase::scoped_ptr<processor_info []>           info_by_procs_;
};

} // namespace rpc
} // namespace mgcom

