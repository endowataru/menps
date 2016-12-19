
#pragma once

#include <mgbase/crtp_base.hpp>
#include <mgbase/utility/move.hpp>
#include <mgbase/assert.hpp>

namespace mgdsm {

template <typename Policy>
class basic_rpc_manager_page_entry
    : public mgbase::crtp_base<Policy>
{
    typedef typename Policy::mutex_type             mutex_type;
    typedef typename Policy::cv_type                cv_type;
    
    typedef typename Policy::owner_plptr_type       owner_plptr_type;
    
    typedef typename Policy::process_id_type        process_id_type;
    typedef typename Policy::process_id_set_type    process_id_set_type;
    
    typedef typename Policy::invalidator_type       invalidator_type;
    
public:
    // unique_lock_type is used in rpc_manager_page_accessor.
    typedef typename Policy::unique_lock_type       unique_lock_type;
    
    unique_lock_type get_lock()
    {
        return unique_lock_type(mtx_);
    }
    
    void wait_if_migrating(unique_lock_type& lk)
    {
        MGBASE_ASSERT(lk.owns_lock());
        
        while (this->is_migrating())
        {
            this->migrate_cv_.wait(lk);
        }
    }
    
    owner_plptr_type get_owner_plptr() const MGBASE_NOEXCEPT {
        return this->owner_plptr_;
    }
    
    void add_reader(const process_id_type proc) {
        this->readers_.insert(proc);
    }
    void remove_reader(const process_id_type proc) {
        this->readers_.erase(proc);
    }
    
    void add_writer(const process_id_type proc) {
        this->writers_.insert(proc);
    }
    void remove_writer(const process_id_type proc) {
        this->writers_.erase(proc);
    }
    
    bool is_readonly() const {
        return this->writers_.empty();
    }
    
    bool is_written_by(const process_id_type proc) {
        return this->writers_.includes(proc);
    }
    
    bool is_only_written_by(const process_id_type proc) {
        return this->writers_.is_only(proc);
    }
    
    invalidator_type make_invalidator()
    {
        struct conf {
            const process_id_set_type&  readers;
            const process_id_set_type&  writers;
        };
        return invalidator_type(conf{this->readers_, this->writers_});
    }
    
    #if 0
    
    bool is_flush_needed(const process_id_type reader_proc) const MGBASE_NOEXCEPT
    {
        return !(
            // Flush is not necessary when:
            
            // If there is no writer
            this->writers_.empty()
            // or
            || (
                // there is a writer
                this->writers_.size() == 1
                // but it is the same process as the new reader.
                && *this->writers_.begin() == reader_proc
            )
        );
    }
    
    bool is_diff_needed(const process_id_type writer_proc) const MGBASE_NOEXCEPT
    {
        // FIXME
        return writers_.size() > 1;
    }
    
    bool was_single_writer(const process_id_type writer_proc) const MGBASE_NOEXCEPT
    {
        return writers_.size() == 1 && *writers_.begin() != writer_proc;
    }
    #endif
    
private:
    bool is_migrating()
    {
        return
            // If the page is invalid, it should be first-touch or migrating.
            Policy::is_invalid_plptr(owner_plptr_)
            // During a migration, there must be at least one writer continuing the migration.
            && this->writers_.empty();
    }
    
    mutex_type mtx_;
    
    cv_type migrate_cv_;
    
    // The address of backing store page.
    owner_plptr_type owner_plptr_;
    
    // Processors reading as a "read-only" page.
    process_id_set_type readers_;
    
    // Processors writing the page.
    process_id_set_type writers_;
};

} // namespace mgdsm

