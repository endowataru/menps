
#pragma once

#include <menps/medsm3/common.hpp>

namespace menps {
namespace medsm3 {

template <typename P>
class blk_local_lock
{
    using blk_id_type = typename P::blk_id_type;
    using seg_id_type = typename P::seg_id_type;
    using blk_subindex_type = typename P::blk_subindex_type;

    using com_itf_type = typename P::com_itf_type;
    using ult_itf_type = typename P::ult_itf_type;
    using blk_mutex_type = typename P::blk_mutex_type;
    using blk_unique_lock_type = typename ult_itf_type::template unique_lock<blk_mutex_type>;
    using lock_tag_type = typename P::lock_tag_type;

public:
    explicit blk_local_lock(
        com_itf_type&           com
    ,   const blk_id_type       blk_id
    ,   blk_unique_lock_type    blk_ulk
    ,   const fdn::size_t       blk_size
    ,   const lock_tag_type     lk_tag
    ) noexcept
        : com_(com)
        , blk_id_(blk_id) // Note: {} cannot be used in GCC 4.8?
        , blk_ulk_{fdn::move(blk_ulk)}
        , blk_size_{blk_size}
        , lk_tag_{lk_tag}
    {
        CMPTH_P_ASSERT(P, P::is_valid_blk_id(blk_id));
        CMPTH_P_ASSERT(P, this->blk_ulk_.owns_lock());
        CMPTH_P_ASSERT(P, this->blk_size_ > 0);

        CMPTH_P_LOG_DEBUG(P
        ,   "Locked block."
        ,   "blk_id", blk_id.to_str()
        ,   "blk_size", blk_size
        ,   "lock_tag", lk_tag
        );
    }

    blk_local_lock(blk_local_lock&&) noexcept = default;
    //blk_local_lock& operator = (blk_local_lock&&) noexcept = default;

    ~blk_local_lock() {
        if (this->blk_ulk_.owns_lock()) {
            CMPTH_P_LOG_DEBUG(P
            ,   "Unlock block."
            ,   "blk_id", this->blk_id().to_str()
            ,   "blk_size", this->blk_size()
            ,   "lock_tag", this->lock_tag()
            );
        }
    }

    com_itf_type& get_com_itf() { return this->com_; }

    blk_id_type blk_id() const noexcept { return this->blk_id_; }
    seg_id_type seg_id() const noexcept { return this->blk_id().seg_id; }
    blk_subindex_type subindex() const noexcept { return this->blk_id().sidx; }
    fdn::size_t blk_size() const noexcept { return this->blk_size_; }
    lock_tag_type lock_tag() const noexcept { return this->lk_tag_; }

private:
    com_itf_type&           com_;
    blk_id_type             blk_id_ = blk_id_type();
    blk_unique_lock_type    blk_ulk_;
    fdn::size_t             blk_size_ = 0;
    lock_tag_type           lk_tag_ = 0;
};

} // namespace medsm3
} // namespace menps

