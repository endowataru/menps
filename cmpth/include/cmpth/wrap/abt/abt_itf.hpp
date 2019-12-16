
#pragma once

#include <cmpth/wrap/abt/abt_thread.hpp>
#include <cmpth/wrap/abt/abt_global.hpp>
#include <cmpth/wrap/abt/abt_pool.hpp>
#include <cmpth/wrap/abt/abt_xstream.hpp>
#include <cmpth/wrap/abt/abt_mutex.hpp>
#include <cmpth/wrap/abt/abt_barrier.hpp>
#include <cmpth/wrap/atomic_itf_base.hpp>
#include <cmpth/ult_tag.hpp>

namespace cmpth {

struct abt_base_policy
{
    using assert_aspect_type = def_assert_aspect;
    using log_aspect_type = def_log_aspect;
    using constants_type = constants;
};

struct abt_global_policy : abt_base_policy
{
    using pool_type = abt_pool<abt_base_policy>;
    using xstream_type = abt_xstream<abt_base_policy>;
};

// level 1

struct lv1_abt_itf
    : atomic_itf_base
{
    using assert_aspect = abt_base_policy::assert_aspect_type;
    using log_aspect = abt_base_policy::log_aspect_type;

    using global = abt_global<abt_global_policy>;
    
    class initializer {
    public:
        explicit initializer(const int argc, char** const argv) {
            global::init(argc, argv);
        }
        ~initializer() /*noexcept*/ {
            global::finalize();
        }

        initializer(const initializer&) = delete;
        initializer& operator = (const initializer&) = delete;
    };

    using worker_num_type = fdn::size_t;
    static worker_num_type get_worker_num() noexcept {
        int rank = 0;
        abt_error::check_error(ABT_xstream_self_rank(&rank));
        return rank;
    }
    static worker_num_type get_num_workers() noexcept {
        int num_xstreams = 0;
        abt_error::check_error(ABT_xstream_get_num(&num_xstreams));
        return static_cast<worker_num_type>(num_xstreams);
    }
};

// level 5

struct abt_itf_thread_policy
    : abt_base_policy
{
    static ABT_pool get_pool() {
        const auto wk_num = lv1_abt_itf::get_worker_num();
        return lv1_abt_itf::global::get_instance().get_pool(wk_num);
    }
};

struct lv5_abt_itf
    : lv1_abt_itf
{
    using thread = abt_thread<abt_itf_thread_policy>;
};

// level 7

struct lv7_abt_itf
    : lv5_abt_itf
{
    using barrier = abt_barrier<abt_base_policy>;
    using mutex = abt_mutex<abt_base_policy>;
};

using abt_itf = lv7_abt_itf;


template <>
struct get_ult_itf_type<ult_tag_t::ABT>
    : fdn::type_identity<abt_itf> { };

} // namespace cmpth

