
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
    // TODO: Are these functions correct?
    void assign_reader(const owner_plptr_type& owner) {
        update(owner);
    }
    void assign_writer(const owner_plptr_type& owner)
    {
        //MGBASE_ASSERT(pg_ent.is_migrating());
        update(owner);
    }
    
    void update(const owner_plptr_type& owner)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // Update the owner process and address.
        // It also notifies all of the readers waiting for the new address.
        pg_ent.update_owner(owner);
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
        const auto owner_plptr = pg_ent.acquire_owner_plptr();
        
        // Determine whether this page is needed to be "flush"ed.
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
        bool                needs_flush;
        bool                needs_diff;
        invalidator_type    inv;
    };
    
    acquire_write_result acquire_write(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // The new writer must not be already a writer.
        MGBASE_ASSERT(! pg_ent.is_written_by(proc));
        
        // Publish the owner ID and address.
        const auto owner_plptr = pg_ent.acquire_owner_plptr();
        
        // Determine whether the new writer needs to write via "flush".
        const bool needs_flush = self.is_flush_needed(proc);
        
        // Determine whether the new writer needs to write via "diff".
        const bool needs_diff = self.is_diff_needed(proc);
        
        // Copy the readers/writers and make an invalidator object.
        auto inv = pg_ent.make_invalidator();
        
        // Add the process as a writer.
        pg_ent.add_writer(proc);
        
        return {
            owner_plptr
        ,   needs_flush
        ,   needs_diff
        ,   mgbase::move(inv)
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
    // TODO: "needs_flush" and "needs_diff" are the same!!!
    
    bool is_flush_needed(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // "proc" is not writing this page.
        // If there is a writer or more, they should be different writers.
        
        return !pg_ent.is_readonly();
    }
    
    bool is_diff_needed(const process_id_type proc)
    {
        auto& self = this->derived();
        auto& pg_ent = self.get_page_entry();
        
        // "proc" is not writing this page.
        // If there is a writer or more, they should be different writers.
        
        return !pg_ent.is_readonly();
    }
};

} // namespace mgdsm

