
#pragma once

#include "basic_sharer_block_accessor.hpp"
#include "sharer_block.hpp"
#include "sharer_page_accessor.hpp"
#include "sharer_block_transfer.hpp"
#include <mgcom/common_policy.hpp>
#include <mgcom/rpc/rpc_policy.hpp>

namespace mgdsm {

struct sharer_block_accessor_traits
    : mgcom::common_policy
    , mgcom::rpc::rpc_policy
{
    typedef sharer_block::accessor  derived_type;
    
    typedef mgbase::uintptr_t       index_type;
    
    static const mgcom::rpc::handler_id_t write_diff_handler_id = 601;
};

class sharer_block::accessor
    : public basic_sharer_block_accessor<sharer_block_accessor_traits>
    , public sharer_block_transfer<sharer_block_accessor_traits>
{
    typedef sharer_page::accessor   page_accessor_type;
    typedef sharer_block_entry      block_entry_type;
    typedef block_id_t              block_id_type;
    
public:
    explicit accessor(sharer_page::accessor& pg_pr, const block_id_t blk_id, sharer_block& blk)
        : pg_pr_(pg_pr)
        , blk_id_(blk_id)
        , blk_(blk)
    { }
    
    page_accessor_type& get_page_accessor() const MGBASE_NOEXCEPT {
        return pg_pr_;
    }
    block_id_type get_block_id() const MGBASE_NOEXCEPT {
        return blk_id_;
    }
    
    // Transfer functions
    page_id_t get_page_id() const MGBASE_NOEXCEPT {
        return this->get_page_accessor().get_page_id();
    }
    page_id_t get_segment_id() const MGBASE_NOEXCEPT {
        return this->get_page_accessor().get_segment_accessor().get_segment_id();
    }
    abs_block_id get_abs_block_id() const MGBASE_NOEXCEPT {
        return {
            this->get_segment_id()
        ,   this->get_page_id()
        ,   this->get_block_id()
        };
    }
    
    std::string to_string() const
    {
        return fmt::format(
            "{}\t"
            "blk_id:{}"
        ,   pg_pr_.to_string()
        ,   blk_id_
        );
    }
    
private:
    friend class basic_sharer_block_accessor<sharer_block_accessor_traits>;
    
    block_entry_type& get_block_entry() const MGBASE_NOEXCEPT {
        return blk_;
    }
    
    page_accessor_type& pg_pr_;
    block_id_type       blk_id_;
    block_entry_type&   blk_;
};

sharer_block::accessor sharer_page::accessor::get_block_accessor(const block_id_t blk_id) MGBASE_NOEXCEPT
{
    auto& blk = this->pg_->get_block(blk_id);
    
    return sharer_block::accessor(*this, blk_id, blk);
}

} // namespace mgdsm

