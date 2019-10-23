
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class basic_wr_count_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_base_type = typename P::blk_global_lock_base_type;

public:
    bool needs_local_copy(blk_global_lock_base_type& blk_glk)
    {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();

        const auto wr_count = self.get_wr_count(blk_llk);
        const auto wr_count_threshold = self.get_wr_count_threshold(blk_llk);

        return self.is_lazy_merge_enabled()
            ? (wr_count+2) >= wr_count_threshold
            : true;
    }

    bool needs_local_comp(blk_global_lock_base_type& blk_glk)
    {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();

        const auto wr_count = self.get_wr_count(blk_llk);
        const auto wr_count_threshold = self.get_wr_count_threshold(blk_llk);
        
        return self.is_needs_local_comp_enabled()
            ? (wr_count+1) >= wr_count_threshold
            : true;
    }

    bool is_fast_released(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();

        if (self.is_fast_release_enabled()) {
            const auto fast_rel_count = self.get_fast_rel_count(blk_llk);
            const auto fast_rel_threshold = self.get_fast_rel_threshold(blk_llk);
            return fast_rel_count < fast_rel_threshold;
        }
        else {
            return false;
        }
    }

    bool needs_protect_before(blk_global_lock_base_type& blk_glk) {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();

        const auto wr_count = self.get_wr_count(blk_llk);
        const auto wr_count_threshold = self.get_wr_count_threshold(blk_llk);

        return wr_count >= wr_count_threshold;
    }

    template <typename BeginRet, typename MergeRet>
    void update_global_end(blk_global_lock_base_type& blk_glk, const BeginRet& bt_ret, const MergeRet& mg_ret)
    {
        auto& self = this->derived();
        auto& blk_llk = blk_glk.local_lock();

        const auto needs_local_comp_ret = this->needs_local_comp(blk_glk);
        auto wr_count = self.get_wr_count(blk_llk);

        if (self.is_fast_release_enabled()) {
            auto fast_rel_threshold = self.get_fast_rel_threshold(blk_llk);
            if (bt_ret.is_write_protected) {
                // Reset the counts.
                wr_count = 0;
                fast_rel_threshold = 1;
            }
            else if (needs_local_comp_ret && mg_ret.is_written) {
                // The block is written.
                wr_count = 0;
                fast_rel_threshold =
                    std::min(
                        fast_rel_threshold * 2
                    ,   self.get_max_fast_rel_threshold(blk_llk)
                    );
            }
            else {
                auto fast_rel_count = self.get_fast_rel_count(blk_llk);
                // The block is not modified.
                wr_count += fast_rel_count + 1;
                fast_rel_threshold = 1;
            }
            self.set_fast_rel_count(blk_llk, 0);
            // TODO: May overflow.
            self.set_fast_rel_threshold(blk_llk, fast_rel_threshold);
        }
        else {
            // Update the write count.
            wr_count =
                (bt_ret.is_write_protected ||
                    (bt_ret.needs_local_comp && mg_ret.is_written))
                ? 0 : (wr_count + 1);
            // TODO: May overflow.
        }

        self.set_wr_count(blk_llk, wr_count);
    }
};

} // namespace medsm3
} // namespace menps

