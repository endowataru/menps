
#pragma once

#include "medsm_common.hpp"
#include <menps/mecom/rma/paired_local_ptr.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medsm {

class manager_segment_proxy
{
    typedef page_id_t                           page_id_type;
    typedef mecom::rma::paired_local_ptr<void>  plptr_type;
    
public:
    virtual ~manager_segment_proxy() /*noexcept*/ = default;
    
    virtual mefdn::size_t get_page_size() const = 0;
    
    virtual mefdn::size_t get_num_pages() const = 0;
    
    virtual mefdn::size_t get_block_size() const = 0;
    
    struct acquire_read_result
    {
        plptr_type  owner_plptr;
        bool        needs_flush;
    };
    
    MEFDN_NODISCARD
    virtual acquire_read_result acquire_read(page_id_type) = 0;
    
    virtual void release_read(page_id_type) = 0;
    
    struct acquire_write_result
    {
        plptr_type  owner_plptr;
        bool        needs_flush;
        bool        needs_diff;
    };
    
    MEFDN_NODISCARD
    virtual acquire_write_result acquire_write(page_id_type) = 0;
    
    struct release_write_result
    {
        bool        needs_flush;
    };
    
    MEFDN_NODISCARD
    virtual release_write_result release_write(page_id_type) = 0;
    
    virtual void assign_reader(page_id_type, const plptr_type&) = 0;
    
    virtual void assign_writer(page_id_type, const plptr_type&) = 0;
};

typedef mefdn::unique_ptr<manager_segment_proxy>   manager_segment_proxy_ptr;

} // namespace medsm
} // namespace menps

