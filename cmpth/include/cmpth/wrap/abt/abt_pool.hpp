
#pragma once

#include <cmpth/wrap/abt/abt.hpp>

namespace cmpth {

struct abt_pool_deleter {
    void operator() (ABT_pool pool) const noexcept {
        ABT_pool_free(&pool);
    }
};

template <typename P>
class abt_pool
    : public fdn::unique_ptr<ABT_pool_opaque, abt_pool_deleter>
{
    using base = fdn::unique_ptr<ABT_pool_opaque, abt_pool_deleter>;

public:
    abt_pool() noexcept = default;

    explicit abt_pool(ABT_pool pool)
        : base{pool}
    { }

    abt_pool(abt_pool&&) noexcept = default;
    abt_pool& operator = (abt_pool&&) noexcept = default;

    static abt_pool create_basic(
        const ABT_pool_kind     kind
    ,   const ABT_pool_access   access
    ,   const ABT_bool          automatic
    ) {
        ABT_pool pool = ABT_POOL_NULL;
        abt_error::check_error(
            ABT_pool_create_basic(kind, access, automatic, &pool)
        );
        return abt_pool{pool};
    }
};

} // namespace cmpth

