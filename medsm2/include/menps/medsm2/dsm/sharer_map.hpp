
#pragma once

#include <menps/medsm2/common.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class sharer_map
{
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;

    using size_type = typename P::size_type;

public:
    explicit sharer_map(size_type size)
        : bm_(size, 0)
    { }
    
    void set(const proc_id_type proc_id) noexcept {
        this->bm_[proc_id >> 3] |= 1 << (proc_id & 0x7);
    }
    void unset(const proc_id_type proc_id) noexcept {
        this->bm_[proc_id >> 3] &= 0xFF << (proc_id & 0x7);
    }
    bool is_set(const proc_id_type proc_id) const noexcept {
        return (this->bm_[proc_id >> 3] >> (proc_id & 0x7)) & 1;
    }

    static sharer_map copy_from(const mefdn::byte* const bm, const size_type size)
    {
        sharer_map ret{size};
        std::memcpy(&ret.bm_[0], bm, size);
        return ret;
    }

    mefdn::byte* get_raw() noexcept {
        return reinterpret_cast<mefdn::byte*>(&this->bm_[0]);
    }
    const mefdn::byte* get_raw() const noexcept {
        return reinterpret_cast<const mefdn::byte*>(&this->bm_[0]);
    }
    
    static constexpr size_type get_sharer_map_size(proc_id_type num_procs) noexcept {
        return ((num_procs - 1) >> 3) + 1;
    }

private:
    std::vector<unsigned char> bm_;
};

} // namespace medsm2
} // namespace menps

