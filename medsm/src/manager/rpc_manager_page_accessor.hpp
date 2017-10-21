
#pragma once

#include "basic_rpc_manager_page_accessor.hpp"
#include "rpc_manager_page.hpp"
#include "rpc_manager_segment_accessor.hpp"
#include "rpc_manager_page_invalidator.hpp"

namespace menps {
namespace medsm {

struct rpc_manager_page_accessor_policy
{
    typedef rpc_manager_page::accessor          derived_type;
    
    typedef mecom::rma::paired_local_ptr<void>  owner_plptr_type;
    typedef mecom::process_id_t                 process_id_type;
    
    typedef rpc_manager_page_invalidator        invalidator_type;
};

class rpc_manager_page::accessor
    : public basic_rpc_manager_page_accessor<rpc_manager_page_accessor_policy>
{
    typedef basic_rpc_manager_page_accessor<rpc_manager_page_accessor_policy> base;
    
    typedef rpc_manager_page_accessor_policy::invalidator_type  invalidator_type;
    
public:
    accessor(
        rpc_manager_segment::accessor&  seg_ac
    ,   rpc_manager_page&               pg
    )
        : seg_ac_(&seg_ac)
        , pg_(&pg)
        , lk_(pg.get_lock())
    { }
    
    accessor(const accessor&) = delete;
    accessor& operator = (const accessor&) = delete;
    
    accessor(accessor&&) noexcept = default;
    accessor& operator = (accessor&&) noexcept = default;
    
private:
    friend class basic_rpc_manager_page_accessor<rpc_manager_page_accessor_policy>;
    
    rpc_manager_page_entry& get_page_entry() noexcept {
        return *pg_;
    }
    void wait_if_migrating() {
        pg_->wait_if_migrating(lk_);
    }
    
    rpc_manager_segment::accessor*              seg_ac_;
    rpc_manager_page*                           pg_;
    rpc_manager_page_entry::unique_lock_type    lk_;
};

rpc_manager_page::accessor rpc_manager_segment::accessor::get_page_accessor(const page_id_t pg_id)
{
    auto& pg = this->seg_.get_page(pg_id);
    
    return rpc_manager_page::accessor(*this, pg);
}

} // namespace medsm
} // namespace menps

