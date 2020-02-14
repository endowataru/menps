
#pragma once

#include <cmpth/wrap/kth/kth_thread.hpp>
#include <cmpth/wrap/kth/kth_worker_set.hpp>
#include <cmpth/wrap/klt_itf.hpp>
#include <cmpth/sync/basic_cv_barrier.hpp>
#include <cmpth/ult_tag.hpp>
#include <stack>

namespace cmpth {

struct kth_worker_set_policy
{
    using worker_num_type = fdn::size_t;
    using worker_num_set_type = std::stack<worker_num_type>;

    using mutex_type = klt_itf::mutex;
    using lock_guard_type = klt_itf::lock_guard<mutex_type>;

    using assert_aspect_type = def_assert_aspect; // TODO
    using log_aspect_type = def_log_aspect; // TODO
};

struct kth_thread_policy
{
    using base_thread_type = klt_itf::thread;

    using worker_set_type = kth_worker_set<kth_worker_set_policy>;
    static worker_set_type& get_worker_set() {
        static worker_set_type wk_set{get_num_workers()}; // TODO: singleton
        return wk_set;
    }

private:
    static fdn::size_t get_num_workers() {
        const char* const num_wks_str = std::getenv("CMPTH_NUM_WORKERS");
        return static_cast<fdn::size_t>(num_wks_str ? std::atoi(num_wks_str) : 1);
    }
};

struct kth_barrier_policy
{
    using mutex_type = klt_itf::mutex;
    using cv_type = klt_itf::condition_variable;
    using unique_lock_type = klt_itf::unique_lock<mutex_type>;
};

struct kth_itf
    : atomic_itf_base
{
    using thread = kth_thread<kth_thread_policy>;

    using mutex = klt_itf::mutex;
    using condition_variable = klt_itf::condition_variable;
    
    using spinlock = klt_itf::spinlock; // TODO

    using barrier = basic_cv_barrier<kth_barrier_policy>;
    
    template <typename Mutex>
    using lock_guard = klt_itf::lock_guard<Mutex>;
    template <typename Mutex>
    using unique_lock = klt_itf::unique_lock<Mutex>;

    template <typename P>
    using thread_specific = klt_itf::thread_specific<P>;

    using this_thread = klt_itf::this_thread;
    struct initializer {
        initializer() {
            auto& wk_set = kth_thread_policy::get_worker_set();
            wk_set.initialize_worker();
        }
        initializer(int /*argc*/, char** /*argv*/)
            : initializer{} { }

        ~initializer() {
            auto& wk_set = kth_thread_policy::get_worker_set();
            wk_set.finalize_worker();
        }

        initializer(const initializer&) = delete;
        initializer& operator = (const initializer&) = delete;
    };

    using assert_aspect = def_assert_aspect; // TODO
    using log_aspect = def_log_aspect; // TODO

    using worker_num_type = kth_worker_set_policy::worker_num_type;
    static worker_num_type get_worker_num() {
        auto& wk_set = kth_thread_policy::get_worker_set();
        return wk_set.get_worker_num();
    }
    static worker_num_type get_num_workers() {
        auto& wk_set = kth_thread_policy::get_worker_set();
        return wk_set.get_num_workers();
    }
};

template <>
struct get_ult_itf_type<ult_tag_t::KTH>
    : fdn::type_identity<kth_itf> { };

} // namespace cmpth

