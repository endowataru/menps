
#pragma once

#include <menps/mecom/rpc.hpp>
#include <menps/mefdn/threading/spinlock.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/external/fmt.hpp>

namespace menps {
namespace mecom {
namespace rpc {

template <typename T, typename NicT, NicT NumNics>
class rpc_resource_counter
{
public:
    void initialize(const T max_num_tickets)
    {
        max_num_tickets_ = max_num_tickets;
        
        for (NicT local_nic = 0; local_nic < NumNics; ++local_nic)
            local_tickets_[local_nic].store(0, mefdn::memory_order_relaxed);
        
        for (NicT remote_nic = 0; remote_nic < NumNics; ++remote_nic)
            remote_tickets_[remote_nic].store(max_num_tickets, mefdn::memory_order_relaxed);
    }
    
    bool try_start_sending(const NicT remote_nic, T local_tickets_result[NumNics])
    {
        // Get an exclusive lock.
        // This lock is used to ensure the sending order to the remote NIC.
        if (!remote_locks_[remote_nic].try_lock())
            return false;
        
        // Try to acquire one remote ticket.
        
        while (true)
        {
            /*const*/ T ticket = remote_tickets_[remote_nic].load(mefdn::memory_order_relaxed);
            
            if (ticket == 0) {
                // Failed to acquire one remote ticket.
                remote_locks_[remote_nic].unlock(); // Unlock the acquired lock.
                return false;
            }
            
            if (remote_tickets_[remote_nic].compare_exchange_weak(
                ticket      // expected
            ,   ticket - 1  // desired
            ,   mefdn::memory_order_acquire
            ))
                break;
        }
        
        MEFDN_ASSERT(remote_tickets_[remote_nic].load() <= max_num_tickets_);
        
        // Get the local tickets as many as possible.
        
        for (NicT local_nic = 0; local_nic < NumNics; ++local_nic)
        {
            while (true)
            {
                /*const*/ T ticket = local_tickets_[local_nic].load(mefdn::memory_order_relaxed);
                
                if (ticket > 0)
                {
                    // If this thread can send back the local ticket,
                    // then the tickets must be atomically removed from this process.
                    
                    if (!local_tickets_[local_nic].compare_exchange_weak(
                        ticket  // expected
                    ,   0       // desired
                    ,   mefdn::memory_order_acquire
                    ))
                        continue; // Failed to pull the local ticket. Retry.
                }
                
                local_tickets_result[local_nic] = ticket;
                
                break;
            }
        }
        
        MEFDN_LOG_DEBUG(
            "msg:Started sending RPC.\t"
            "remote_nic:{}\t{}"
        ,   remote_nic
        ,   show_tickets(local_tickets_result)
        );
        
        return true;
    }

private:
    static std::string show_tickets(const T tickets[NumNics])
    {
        fmt::MemoryWriter w;
        for (NicT nic = 0; nic < NumNics; ++nic)
            w.write("ticket_nic{}:{}\t", nic, tickets[nic]);
        
        return w.str();
    }

public:
    void finish_sending(const NicT remote_nic)
    {
        remote_locks_[remote_nic].unlock();
        
        MEFDN_LOG_DEBUG("msg:Finished sending RPC.\tremote_nic:{}", remote_nic);
    }
    
    void start_receiving(const T remote_tickets[NumNics])
    {
        // Restore the remote tickets.
        // This operation allows the threads in this process to send messages again.
        for (NicT remote_nic = 0; remote_nic < NumNics; ++remote_nic) {
            remote_tickets_[remote_nic].fetch_add(remote_tickets[remote_nic], mefdn::memory_order_release);
            
            MEFDN_ASSERT(remote_tickets_[remote_nic].load() <= max_num_tickets_);
        }
    }
    
    void finish_receiving(const NicT local_nic)
    {
        // Add a local ticket.
        // This ticket will be sent back when try_send is called.
        local_tickets_[local_nic].fetch_add(1, mefdn::memory_order_relaxed);
        
        MEFDN_ASSERT(local_tickets_[local_nic].load() <= max_num_tickets_);
    }

private:
    T                   max_num_tickets_; // For debugging
    mefdn::atomic<T>   local_tickets_[NumNics];
    mefdn::atomic<T>   remote_tickets_[NumNics];
    mefdn::spinlock    remote_locks_[NumNics];
};

} // namespace rpc
} // namespace mecom
} // namespace menps

