
#pragma once

#include "rpc_resource_counter.hpp"
#include <menps/mecom/rpc.hpp>
#include <menps/mefdn/scoped_ptr.hpp>

namespace menps {
namespace mecom {
namespace rpc {

// Note: This class assumes that each NIC doesn't shuffle the order of the messages.
//       Under this assumption, the buffer resource is simply managed by integer counters.

template <typename TicketT, typename NicT, NicT NumNics>
class rpc_resource_manager
{
    static const NicT number_of_nics = NumNics;
    
public:
    void initialize(endpoint& ep, const TicketT max_num_connections)
    {
        procs_ = new processor_info[ep.number_of_processes()];
        
        for (process_id_t proc = 0; proc < ep.number_of_processes(); ++proc)
            procs_[proc].counter.initialize(max_num_connections);
    }
    void finalize()
    {
        procs_.reset();
    }
    
    bool try_start_sending(
        const process_id_t  receiver_proc
    ,   NicT* const         remote_nic_result
    ,   TicketT             local_tickets_result[number_of_nics]
    ) noexcept
    {
        processor_info& info = procs_[receiver_proc];
        
        const NicT remote_nic_start = info.remote_nic.load(mefdn::memory_order_relaxed);
        
        for (NicT remote_nic = next_nic(remote_nic_start);
            remote_nic != remote_nic_start;
            remote_nic = next_nic(remote_nic)
        ) {
            if (info.counter.try_start_sending(remote_nic, local_tickets_result)) {
                *remote_nic_result = remote_nic;
                
                // Use the next NIC.
                // Overflowing doesn't matter because in fact its remainder is used.
                info.remote_nic.fetch_add(1, mefdn::memory_order_relaxed);
                
                return true;
            }
        }
        
        return false;
    }
    
    void finish_sending(
        const process_id_t  receiver_proc
    ,   const NicT          remote_nic
    ) noexcept
    {
        processor_info& info = procs_[receiver_proc];
        
        info.counter.finish_sending(remote_nic);
    }
    
    void start_receiving(
        const process_id_t  sender_proc
    ,   const TicketT       remote_tickets[number_of_nics]
    ) noexcept
    {
        processor_info& info = procs_[sender_proc];
        
        info.counter.start_receiving(remote_tickets);
    }
    
    void finish_receiving(
        const process_id_t  sender_proc
    ,   const TicketT       local_nic
    ) noexcept
    {
        processor_info& info = procs_[sender_proc];
        
        info.counter.finish_receiving(local_nic);
    }
    
private:
    static NicT next_nic(const NicT nic) {
        return (nic + 1) % number_of_nics;
    }
    
    struct processor_info {
        mefdn::atomic<NicT>                                remote_nic;
        rpc_resource_counter<TicketT, NicT, number_of_nics> counter;
        
        processor_info() {
            remote_nic.store(0, mefdn::memory_order_relaxed);
        }
    };
    
    mefdn::scoped_ptr<processor_info []> procs_;
};

} // namespace am
} // namespace mecom
} // namespace menps

