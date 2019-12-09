
#pragma once

#include <cmpth/wrap/abt/abt.hpp>

namespace cmpth {

struct abt_xstream_deleter
{
    void operator() (const ABT_xstream xstream) {
        if (CMPTH_UNLIKELY(xstream)) { fdn::terminate(); }
    }
};

template <typename P>
class abt_xstream
    : public fdn::unique_ptr<ABT_xstream_opaque, abt_xstream_deleter>
{
    using base = fdn::unique_ptr<ABT_xstream_opaque, abt_xstream_deleter>;

public:
    abt_xstream() noexcept = default;

    explicit abt_xstream(const ABT_xstream xstream)
        : base{xstream}
    { }

    abt_xstream(abt_xstream&&) noexcept = default;
    abt_xstream& operator = (abt_xstream&&) noexcept = default;

    ~abt_xstream() /*noexcept*/ = default;

    static abt_xstream create_basic(
        const ABT_sched_predef  predef
    ,   const int               num_pools
    ,   ABT_pool* const         pools
    ,   ABT_sched_config        config
    ) {
        ABT_xstream xstream = ABT_XSTREAM_NULL;
        abt_error::check_error(
            ABT_xstream_create_basic(
                predef, num_pools, pools, config, &xstream
            )
        );
        return abt_xstream{xstream};
    }

    void start() {
        abt_error::check_error(
            ABT_xstream_start(this->get())
        );
    }

    void join() {
        abt_error::check_error(
            ABT_xstream_join(this->get())
        );
        auto xstream = this->release();
        abt_error::check_error(
            ABT_xstream_free(&xstream)
        );
    }

    static abt_xstream self() {
        ABT_xstream xstream = ABT_XSTREAM_NULL;
        abt_error::check_error(
            ABT_xstream_self(&xstream)
        );
        return abt_xstream{xstream};
    }

    void set_main_sched_basic(
        const ABT_sched_predef  predef
    ,   const int               num_pools
    ,   ABT_pool* const         pools
    ) {
        abt_error::check_error(
            ABT_xstream_set_main_sched_basic(
                this->get(), predef, num_pools, pools
            )
        );
    }
};

} // namespace cmpth

