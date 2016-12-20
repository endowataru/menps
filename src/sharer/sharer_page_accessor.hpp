
#pragma once

#include "basic_sharer_page_accessor.hpp"
#include "sharer_page.hpp"
#include "sharer_segment_accessor.hpp"
#include <mgcom/rma/paired_remote_ptr.hpp>
#include <mgcom/rma/rma_policy.hpp>

namespace mgdsm {

struct sharer_page_accessor_traits
    : mgcom::rma::rma_policy
{
    typedef sharer_page::accessor                      derived_type;
    
    typedef sharer_segment::accessor                   segment_accessor_type;
    
    typedef sharer_page_entry                       page_entry_type;
    
    typedef block_id_t                              block_id_type;
    typedef mgcom::rma::paired_remote_ptr<void>     prptr_type;
    typedef mgcom::rma::paired_local_ptr<void>      plptr_type;
    
    typedef mgbase::ptrdiff_t                       difference_type;
    
    typedef mgbase::uintptr_t                       index_type;
    
    static bool is_invalid_plptr(const plptr_type& plptr) MGBASE_NOEXCEPT {
        return plptr.ptr == MGBASE_NULLPTR;
    }
    
    static plptr_type allocate_page(const mgbase::size_t size_in_bytes) {
        return {
            mgcom::current_process_id()
        ,   mgcom::rma::local_ptr<void>::cast_from(mgcom::rma::untyped::to_address(mgcom::rma::untyped::allocate(size_in_bytes)))
                // TODO: very ugly...
        };
    }
};

class sharer_page::accessor
    : public basic_sharer_page_accessor<sharer_page_accessor_traits>
{
public:
    accessor(sharer_segment::accessor& seg_pr, const page_id_t pg_id, sharer_page& pg)
        : seg_pr_(seg_pr)
        , pg_id_(pg_id)
        , pg_(pg)
        , pg_lk_(pg.get_lock())
    { }
    
    inline sharer_block::accessor get_block_accessor(block_id_t) MGBASE_NOEXCEPT;
    
private:
    friend class basic_sharer_page_accessor<sharer_page_accessor_traits>;
    
    sharer_segment::accessor& get_segment_accessor() const MGBASE_NOEXCEPT {
        return seg_pr_;
    }
    sharer_page_entry& get_page_entry() const MGBASE_NOEXCEPT {
        return pg_;
    }
    page_id_t get_page_id() const MGBASE_NOEXCEPT {
        return pg_id_;
    }
    
    sharer_segment::accessor&           seg_pr_;
    
    page_id_t                           pg_id_;
    sharer_page&                        pg_;
    
    sharer_page_entry::unique_lock_type pg_lk_;
};

sharer_page::accessor sharer_segment::accessor::get_page_accessor(const page_id_t pg_id) MGBASE_NOEXCEPT
{
    auto& pg = this->seg_.get_page(pg_id);
    
    return sharer_page::accessor(*this, pg_id, pg);
}

} // namespace mgdsm

