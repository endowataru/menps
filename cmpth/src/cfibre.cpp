
#include <cmpth/io/basic_io_delegator.hpp>
#include <cmpth/ext/fibre.hpp>
#include <cmpth/ext/cfibre.h>

namespace cmpth {

struct io_consumer_policy
{
    using assert_aspect_type = fibre_itf::assert_aspect;
    using log_aspect_type = fibre_itf::log_aspect;
    using suspended_thread_type = fibre_itf::suspended_thread;
    static const fdn::size_t num_poll_events = 128; // TODO

    static void yield() {
        fibre_itf::this_thread::yield();
    }
};

struct io_delegator_policy {
    using consumer_type = basic_io_consumer<io_consumer_policy>;
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& /*pool*/) {
        return 128; // TODO
    }
};

struct io_producer_policy
{
    using delegator_type = fibre_itf::delegator_t<io_delegator_policy>;
    using log_aspect_type = fibre_itf::log_aspect;
};

using io_producer = basic_io_producer<io_producer_policy>;

io_producer& get_io_producer() {
    static io_producer p;
    return p;
}

} // namespace cmpth


extern "C" {

#define D(dummy, name, tr, num, ...) \
    tr cfibre_##name(CMPTH_EXPAND_PARAMS_TO_PARAMS(num, __VA_ARGS__)) { \
        return name(CMPTH_EXPAND_PARAMS_TO_ARGS(num, __VA_ARGS__)); \
    }

CMPTH_IO_BYPASS_FUNCS(D)

#undef D

#define D(dummy, name, tr, num, ...) \
    tr cfibre_##name(CMPTH_EXPAND_PARAMS_TO_PARAMS(num, __VA_ARGS__)) { \
        auto& p = cmpth::get_io_producer(); \
        return p.execute_##name({ CMPTH_EXPAND_PARAMS_TO_ARGS(num, __VA_ARGS__) }); \
    }

CMPTH_IO_SETUP_FUNCS(D)
CMPTH_IO_DELEGATED_FUNCS(D)

#undef D

} // extern "C"

