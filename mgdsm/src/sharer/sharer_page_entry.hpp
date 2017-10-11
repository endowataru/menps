
#pragma once

#include "basic_sharer_page_entry.hpp"
#include "sharer_block.hpp"
#include <mgbase/threading/unique_lock.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgcom/rma/paired_remote_ptr.hpp>
#include <mgcom/rma/rma_policy.hpp>

namespace mgdsm {

class sharer_page_entry;

struct sharer_page_entry_traits
    : mgcom::rma::rma_policy
{
    typedef sharer_page_entry    derived_type;
    typedef sharer_block         block_type;
    typedef sharer_block_entry   block_entry_type;
    typedef block_id_t           block_id_type;
    typedef block_id_t           block_count_type; // TODO
    typedef mgcom::rma::paired_remote_ptr<void>     prptr_type;
};


class sharer_page_entry
    : public basic_sharer_page_entry<sharer_page_entry_traits>
{
    typedef mgbase::spinlock        lock_type;
    
public:
    typedef mgbase::unique_lock<lock_type>  unique_lock_type;
    
    unique_lock_type get_lock()
    {
        return unique_lock_type(lock_);
    }
    
private:
    lock_type lock_;
};

} // namespace mgdsm

