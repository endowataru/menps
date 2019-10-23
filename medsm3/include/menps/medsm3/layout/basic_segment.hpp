
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_segment
{
    using com_itf_type = typename P::com_itf_type;

    using global_lock_table_type = typename P::global_lock_table_type;

    using blk_mutex_type = typename P::blk_mutex_type;
    using blk_state_type = typename P::blk_state_type;

    using blk_subindex_type = typename P::blk_subindex_type;

public:
    explicit basic_segment(com_itf_type& com, const fdn::size_t num_blks, const fdn::size_t lad_size)
        : num_blks_{num_blks}
    {
        this->glk_tbl_.coll_make(com, num_blks, lad_size);
    }

    // TODO: Make this private
    global_lock_table_type& get_global_lock_table() noexcept { return this->glk_tbl_; }

    fdn::size_t get_num_blks() const noexcept { return this->num_blks_; }

    bool is_valid_subindex(const blk_subindex_type blk_sidx) const noexcept {
        return blk_sidx < this->get_num_blks();
    }

protected:
    struct blk_local_metadata_base
    {
        // from medsm2::blk_dir_table
        
        blk_mutex_type  mtx;
        // The block state indicates the page protection
        // of the local data block.
        blk_state_type  state;

        // Used by rel_sig.
        bool            flag;
    };

private:
    fdn::size_t num_blks_ = 0;
    global_lock_table_type  glk_tbl_;
};

} // namespace medsm3
} // namespace menps

