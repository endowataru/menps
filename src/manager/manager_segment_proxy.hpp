
#pragma once

#include "mgdsm_common.hpp"
#include <mgcom/rma/paired_local_ptr.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class manager_segment_proxy
{
    typedef page_id_t                           page_id_type;
    typedef mgcom::rma::paired_local_ptr<void>  plptr_type;
    
public:
    virtual ~manager_segment_proxy() /*noexcept*/ = default;
    
    virtual mgbase::size_t get_page_size() const = 0;
    
    virtual mgbase::size_t get_num_pages() const = 0;
    
    virtual mgbase::size_t get_block_size() const = 0;
    
    struct acquire_read_result
    {
        plptr_type  owner_plptr;
        bool        needs_flush;
    };
    
    MGBASE_WARN_UNUSED_RESULT
    virtual acquire_read_result acquire_read(page_id_type) = 0;
    
    virtual void release_read(page_id_type) = 0;
    
    struct acquire_write_result
    {
        plptr_type  owner_plptr;
        bool        needs_flush;
        bool        needs_diff;
    };
    
    MGBASE_WARN_UNUSED_RESULT
    virtual acquire_write_result acquire_write(page_id_type) = 0;
    
    virtual void release_write(page_id_type) = 0;
    
    virtual void assign_reader(page_id_type, const plptr_type&) = 0;
    
    virtual void assign_writer(page_id_type, const plptr_type&) = 0;
};

typedef mgbase::unique_ptr<manager_segment_proxy>   manager_segment_proxy_ptr;

} // namespace mgdsm

