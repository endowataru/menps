
#pragma once

#include "basic_rpc_manager_page_accessor.hpp"
#include "rpc_manager_page.hpp"
#include "rpc_manager_segment_accessor.hpp"
#include "rpc_manager_page_invalidator.hpp"

namespace mgdsm {

struct rpc_manager_page_accessor_policy
{
    typedef rpc_manager_page::accessor          derived_type;
    
    typedef mgcom::rma::paired_local_ptr<void>  owner_plptr_type;
    typedef mgcom::process_id_t                 process_id_type;
    
    typedef rpc_manager_page_invalidator        invalidator_type;
};

class rpc_manager_page::accessor
    : public basic_rpc_manager_page_accessor<rpc_manager_page_accessor_policy>
{
    typedef rpc_manager_page_accessor_policy::invalidator_type  invalidator_type;
    
public:
    accessor(
        rpc_manager_segment::accessor&  seg_ac
    ,   rpc_manager_page&               pg
    )
        : seg_ac_(seg_ac)
        , pg_(pg)
    { }
    
private:
    friend class basic_rpc_manager_page_accessor<rpc_manager_page_accessor_policy>;
    
    rpc_manager_page_entry& get_page_entry() MGBASE_NOEXCEPT {
        return pg_;
    }
    void wait_if_migrating() {
        pg_.wait_if_migrating(lk_);
    }
    
    rpc_manager_segment::accessor& seg_ac_;
    rpc_manager_page& pg_;
    rpc_manager_page_entry::unique_lock_type lk_;
};

rpc_manager_page::accessor rpc_manager_segment::accessor::get_page_accessor(const page_id_t pg_id)
{
    auto& pg = this->seg_.get_page(pg_id);
    
    return rpc_manager_page::accessor(*this, pg);
}

} // namespace mgdsm

