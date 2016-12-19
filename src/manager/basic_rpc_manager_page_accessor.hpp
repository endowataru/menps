
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/assert.hpp>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_page_accessor
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::invalidator_type       invalidator_type;
    
    typedef typename Policy::owner_plptr_type       owner_plptr_type;
    
    typedef typename Policy::process_id_type        process_id_type;
    
    
public:
    void update(const owner_plptr_type owner)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        MGBASE_ASSERT(pg_ent.is_migrating());
        
        // Update the owner process and address.
        pg_ent.set_owner(owner);
        
        // Notify all of the readers waiting for the new address.
        self.notify_migrated();
    }
    
    struct acquire_read_result
    {
        owner_plptr_type    owner_plptr;
        bool                needs_flush;
    };
    
    acquire_read_result acquire_read(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Wait if a migration is on going using a condition variable.
        self.wait_if_migrating();
        
        // Add the process as a reader.
        pg_ent.add_reader(proc);
        
        // Publish the owner ID and address.
        const auto owner_plptr = pg_ent.get_owner_plptr();
        
        // 
        const bool needs_flush = self.is_flush_needed(proc);
        
        return { owner_plptr, needs_flush };
    }
    
    void release_read(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Remove the process as a reader.
        pg_ent.remove_reader(proc);
    }
    
    struct acquire_write_result
    {
        owner_plptr_type    owner_plptr;
        bool                needs_diff;
        invalidator_type    inv;
    };
    
    acquire_write_result acquire_write(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        MGBASE_ASSERT(pg_ent.is_written_by(proc));
        
        // Publish the owner ID and address.
        const auto owner_plptr = pg_ent.get_owner_plptr();
        
        // Determine whether the new writer needs to write via "diff".
        const bool needs_diff = self.is_diff_needed(proc);
        
        // Add the process as a writer.
        pg_ent.add_writer(proc);
        
        return {
            owner_plptr
        ,   needs_diff
        ,   pg_ent.make_invalidator()
        };
    }
    
    void release_write(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Remove the process as a writer.
        pg_ent.remove_writer(proc);
    }
    
private:
    bool is_flush_needed(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return !(pg_ent.is_readonly() || pg_ent.is_only_written_by(proc));
    }
    
    bool is_diff_needed(const process_id_type proc)
    {
        // FIXME
        
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        return !(pg_ent.is_readonly() || pg_ent.is_only_written_by(proc));
    }
};

} // namespace mgdsm

        
        #if 0
        // Determines whether the previous writers needs to be notified.
        const bool was_single_writer = self.was_single_writer(proc);
        
        const bool is_migrated = self.should_migrate(readers_, writers_);
        
        process_id_set_type readers;
        
        if (is_migrated) {
            // We predict this page is a single writer at a certain future interval.
            
            // Copy the readers here.
            // TODO: eliminate heap allocation using alloca()
            readers = readers_;
        }
        
        process_id_type writer = Policy::make_invalid_process_id();
        
        if (was_single_writer) {
            // Mark this page as migrating.
            // (Invalidate the address.)
            Policy::set_invalid_plptr(&owner_plptr_);
            
            // Copy the writers here.
            // TODO: eliminate heap allocation using alloca()
            writer = *writers_.begin();
        }
        
        // Add the process as a writer.
        writers_.insert(proc);
        
        // Explicitly unlock here
        // to improve the concurrency for using this entry.
        lk.unlock();
        
        if (needs_diff)
        {
            // Send diff requests to all of the writers.
            self.send_diff_requests_to(writer);
        }
        
        if (is_migrated)
        {
            // Send invalidate messages to all of the readers.
            self.send_invalidations_to(mgbase::move(readers));
            
            // If the pages are invalidated,
            // then the migration can proceed.
        }
        #endif
