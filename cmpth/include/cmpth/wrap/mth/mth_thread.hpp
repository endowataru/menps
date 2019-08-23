
#pragma once

#include <cmpth/wrap/mth/mth.hpp>
#include <cmpth/wrap/basic_wrap_thread.hpp>

namespace cmpth {

template <typename P>
struct mth_thread_policy
{
    using derived_type = basic_wrap_thread<mth_thread_policy>;
    
    using thread_ptr_type = myth_thread_t;
    
    static void check_error(const int err) {
        if (err != 0) { throw mth_error{}; }
    }
    static thread_ptr_type thread_create(const myth_func_t f, void* const p) {
        return myth_create(f, p);
    }
    static void thread_join(const thread_ptr_type t) {
        void* ret = nullptr;
        check_error(myth_join(t, &ret));
        // ret is unused.
    }
    static void thread_detach(const thread_ptr_type t) {
        check_error(myth_detach(t));
    }
    
    using assert_policy_type = typename P::assert_policy_type;
    using log_policy_type = typename P::log_policy_type;
};

template <typename P>
using mth_thread = basic_wrap_thread<mth_thread_policy<P>>;

} // namespace cmpth

