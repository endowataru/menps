
#pragma once

#include "basic_sharer_page_entry.hpp"
#include "sharer_block.hpp"
#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/thread/spinlock.hpp>
#include <menps/mecom/rma/paired_remote_ptr.hpp>
#include <menps/mecom/rma/rma_policy.hpp>

namespace menps {
namespace medsm {

class sharer_page_entry;

struct sharer_page_entry_traits
    : mecom::rma::rma_policy
{
    typedef sharer_page_entry    derived_type;
    typedef sharer_block         block_type;
    typedef sharer_block_entry   block_entry_type;
    typedef block_id_t           block_id_type;
    typedef block_id_t           block_count_type; // TODO
    typedef mecom::rma::paired_remote_ptr<void>     prptr_type;
};


class sharer_page_entry
    : public basic_sharer_page_entry<sharer_page_entry_traits>
{
    typedef mefdn::spinlock        lock_type;
    
public:
    typedef mefdn::unique_lock<lock_type>  unique_lock_type;
    
    unique_lock_type get_lock()
    {
        return unique_lock_type(lock_);
    }
    
private:
    lock_type lock_;
};

} // namespace medsm
} // namespace menps

