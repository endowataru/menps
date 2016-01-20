
#pragma once

#include <mgcom/am/untyped.hpp>

namespace mgcom {
namespace am {

namespace /*unnamed*/ {

class resource_manager
{
protected:
    static const index_t max_num_tickets = 16;
    
    resource_manager() { }
    
    void initialize()
    {
        local_tickets_ = new index_t[number_of_processes()];
        std::fill(local_tickets_.get(), local_tickets_.get() + number_of_processes(), 0);
        
        remote_tickets_ = new index_t[number_of_processes()];
        std::fill(remote_tickets_.get(), remote_tickets_.get() + number_of_processes(), max_num_tickets);
    }
    
    void finalize()
    {
        local_tickets_.reset();
    }
    
    void push_local_ticket_from(process_id_t src_proc) {
        MGBASE_LOG_DEBUG("msg:Removed ticket.\tsrc:{}\tbefore:{}", src_proc, local_tickets_[src_proc]);
        ++local_tickets_[src_proc];
    }
    
    index_t pull_local_tickets_from(process_id_t src_proc)
    {
        MGBASE_ASSERT(valid_process_id(src_proc));
        MGBASE_LOG_DEBUG("msg:Pulled ticket.\tsrc:{}\tdiff:{}", src_proc, local_tickets_[src_proc]);
        
        index_t result = local_tickets_[src_proc];
        local_tickets_[src_proc] = 0;
        return result;
    }
    
    void restore_remote_tickets_to(process_id_t dest_proc, index_t ticket)
    {
        MGBASE_ASSERT(valid_process_id(dest_proc));
        MGBASE_ASSERT(ticket <= max_num_tickets);
        
        MGBASE_LOG_DEBUG("msg:Added ticket.\tdest:{}\tbefore:{}\tdiff:{}", dest_proc, remote_tickets_[dest_proc], ticket);
        remote_tickets_[dest_proc] += ticket;
    }
    
    bool try_use_remote_ticket_to(process_id_t dest_proc)
    {
        // FIXME: atomic operations
        
        index_t& ticket = remote_tickets_[dest_proc];
        if (ticket <= 0)
            return false;
        
        --ticket;
        return true;
    }

private:
    mgbase::scoped_ptr<index_t []> local_tickets_;
    mgbase::scoped_ptr<index_t []> remote_tickets_;
};

const index_t resource_manager::max_num_tickets;

} // unnamed namespace

} // namespace am
} // namespace mgcom

