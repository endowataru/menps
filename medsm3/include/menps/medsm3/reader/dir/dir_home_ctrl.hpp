
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class dir_home_ctrl
{
    CMPTH_DEFINE_DERIVED(P)

    using blk_local_lock_type = typename P::blk_local_lock_type;
    using blk_global_lock_type = typename P::blk_global_lock_type;

    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using global_entry_type = typename P::global_entry_type;
    using sharer_map_type = typename P::sharer_map_type;

public:
    blk_global_lock_type get_global_lock(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();
        auto lk_ret = this->lock_global(blk_llk);
        return blk_global_lock_type{
            blk_llk, lk_ret.owner_proc, self, fdn::move(lk_ret.ge)
        };
    }

    struct lock_global_result {
        proc_id_type        owner_proc;
        global_entry_type   ge;
    };

    lock_global_result lock_global(blk_local_lock_type& blk_llk)
    {
        auto& self = this->derived();

        auto glk_ret = self.lock_global_raw(blk_llk);

        auto& com = blk_llk.get_com_itf();
        const auto sh_map_size = this->get_sharer_map_size(com);
        CMPTH_P_ASSERT(P, glk_ret.lad_size == sizeof(proc_id_type) + sh_map_size);

        const auto owner_proc = glk_ret.owner_proc;
        const fdn::byte* const lad_ptr = glk_ret.owner_lad.get();

        proc_id_type last_writer_proc = 0;
        std::memcpy(&last_writer_proc, lad_ptr, sizeof(proc_id_type));

        // Load as a sharer map.
        auto owner_sh_map =
            sharer_map_type::copy_from(&lad_ptr[sizeof(proc_id_type)], sh_map_size);

        return { owner_proc, { last_writer_proc, fdn::move(owner_sh_map) } };
    }

    void unlock_global(blk_local_lock_type& blk_llk, const global_entry_type& ge)
    {
        auto& self = this->derived();
        
        auto& com = blk_llk.get_com_itf();
        const auto sh_map_size = this->get_sharer_map_size(com);

        std::vector<fdn::byte> lad_buf(sizeof(proc_id_type) + sh_map_size);
        std::memcpy(lad_buf.data(), &ge.last_writer_proc, sizeof(proc_id_type));
        std::memcpy(&lad_buf[sizeof(proc_id_type)], ge.sh_map.get_raw(), sh_map_size);

        self.unlock_global_raw(blk_llk, lad_buf.data());
    }

private:
    static fdn::size_t get_sharer_map_size(com_itf_type& com) noexcept {
        return sharer_map_type::get_sharer_map_size(com.get_num_procs());
    }
};

} // namespace medsm3
} // namespace menps

