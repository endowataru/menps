
#pragma once

#include <mgcom/rma.hpp>
#include <mgbase/threading/atomic.hpp>
#include <algorithm>
#include "basic_am.hpp"

namespace mgcom {
namespace am {
namespace sender {

namespace /*unnamed*/ {

class basic_impl
{
protected:
    void initialize()
    {
        tickets_ = new mgbase::atomic<index_t>[number_of_processes()];
        std::fill(tickets_, tickets_ + number_of_processes(), constants::max_num_tickets);
    }
    
    void finalize()
    {
        delete[] tickets_;
    }
    
    bool get_ticket_to(process_id_t dest_proc)
    {
        while (true) {
            // TODO: Relaxing memory_order_seq_cst
            index_t ticket = tickets_[dest_proc].load(mgbase::memory_order_relaxed);
            if (ticket == 0)
                return false;
            
            // TODO: Relaxing memory_order_seq_cst
            if (tickets_[dest_proc].compare_exchange_weak(ticket, ticket - 1, mgbase::memory_order_seq_cst))
                return true;
        }
    }
    
    void add_ticket(process_id_t dest_proc, index_t ticket)
    {
        // TODO: Relaxing memory_order_seq_cst
        tickets_[dest_proc].fetch_add(ticket, mgbase::memory_order_seq_cst);
    }
    
private:
    mgbase::atomic<index_t>* tickets_;
};

} // unnamed namespace

} // namespace sender
} // namespace am
} // namespace mgcom

