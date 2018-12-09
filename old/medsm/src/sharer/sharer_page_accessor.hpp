
#pragma once

#include "basic_sharer_page_accessor.hpp"
#include "sharer_page.hpp"
#include "sharer_segment_accessor.hpp"
#include <menps/mecom/rma/paired_remote_ptr.hpp>
#include <menps/mecom/rma/rma_policy.hpp>

namespace menps {
namespace medsm {

struct sharer_page_accessor_traits
    : mecom::rma::rma_policy
{
    typedef sharer_page::accessor                      derived_type;
    
    typedef sharer_segment::accessor                   segment_accessor_type;
    
    typedef sharer_page_entry                       page_entry_type;
    
    typedef block_id_t                              block_id_type;
    typedef mecom::rma::paired_remote_ptr<void>     prptr_type;
    typedef mecom::rma::paired_local_ptr<void>      plptr_type;
    
    typedef mefdn::ptrdiff_t                       difference_type;
    
    typedef mefdn::uintptr_t                       index_type;
    
    static bool is_invalid_plptr(const plptr_type& plptr) noexcept {
        return plptr.ptr == nullptr;
    }
    
    static plptr_type allocate_page(const mefdn::size_t size_in_bytes) {
        return {
            mecom::current_process_id()
        ,   mecom::rma::local_ptr<void>::cast_from(mecom::rma::untyped::to_address(mecom::rma::untyped::allocate(size_in_bytes)))
                // TODO: very ugly...
        };
    }
};

class sharer_page::accessor
    : public basic_sharer_page_accessor<sharer_page_accessor_traits>
{
    typedef basic_sharer_page_accessor<sharer_page_accessor_traits> base;
public:
    explicit accessor(sharer_segment::accessor& seg_pr, const page_id_t pg_id, sharer_page& pg)
        : seg_pr_(&seg_pr)
        , pg_id_(pg_id)
        , pg_(&pg)
        , pg_lk_(pg.get_lock())
    { }
    
    accessor(const accessor&) = delete;
    accessor& operator = (const accessor&) = delete;
    
    accessor(accessor&&) noexcept = default;
    accessor& operator = (accessor&&) noexcept = default;
    
    // defined in sharer_block_accessor.hpp
    inline sharer_block::accessor get_block_accessor(block_id_t) noexcept;
    
    sharer_segment::accessor& get_segment_accessor() const noexcept {
        return *seg_pr_;
    }
    page_id_t get_page_id() const noexcept {
        return pg_id_;
    }
    
    std::string to_string() const
    {
        return fmt::format(
            "{}\t"
            "pg_id:{}"
        ,   seg_pr_->to_string()
        ,   pg_id_
        );
    }
    
private:
    friend class basic_sharer_page_accessor<sharer_page_accessor_traits>;
    
    sharer_page_entry& get_page_entry() const noexcept {
        return *pg_;
    }
    
    sharer_segment::accessor*           seg_pr_;
    
    page_id_t                           pg_id_;
    sharer_page*                        pg_;
    
    sharer_page_entry::unique_lock_type pg_lk_;
};

sharer_page::accessor sharer_segment::accessor::get_page_accessor(const page_id_t pg_id) noexcept
{
    auto& pg = this->seg_.get_page(pg_id);
    
    return sharer_page::accessor(*this, pg_id, pg);
}

} // namespace medsm
} // namespace menps

