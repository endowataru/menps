
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
    basic_rpc_manager_page_entry()
        : owner_plptr_(Policy::make_invalid_plptr())
        , is_migrating_(false)
    { }
    
    // unique_lock_type is used in rpc_manager_page_accessor.
    typedef typename Policy::unique_lock_type       unique_lock_type;
    
    unique_lock_type get_lock()
    {
        return unique_lock_type(mtx_);
    }
    
private:
    void start_migrating()
    {
        MGBASE_ASSERT(! this->is_migrating());
        
        this->is_migrating_ = true;
    }
    void notify_migrated()
    {
        this->is_migrating_ = false;
        
        this->migrate_cv_.notify_all();
    }
    
public:
    void wait_if_migrating(unique_lock_type& lk)
    {
        MGBASE_ASSERT(lk.owns_lock());
        
        while (this->is_migrating())
        {
            this->migrate_cv_.wait(lk);
        }
    }
    
    owner_plptr_type acquire_owner_plptr() MGBASE_NOEXCEPT
    {
        // The address of the migrating page is not allowed to be retrieved.
        MGBASE_ASSERT(!this->is_migrating());
        
        if (Policy::is_invalid_plptr(this->owner_plptr_))
        {
            // If the page address is invalid,
            // this page must be migrated.
            
            this->start_migrating();
        }
        
        return this->owner_plptr_;
    }
    void update_owner(const owner_plptr_type& plptr)
    {
        this->owner_plptr_ = plptr;
        
        this->notify_migrated();
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
    
private:
    bool is_migrating()
    {
        return this->is_migrating_;
    }
    
    mutex_type mtx_;
    
    cv_type migrate_cv_;
    
    // The address of backing store page.
    owner_plptr_type owner_plptr_;
    
    bool is_migrating_;
    
    // Processors reading as a "read-only" page.
    process_id_set_type readers_;
    
    // Processors writing the page.
    process_id_set_type writers_;
};

} // namespace mgdsm

