
#pragma once

#include <cmpth/wrap/mth/mth_itf.hpp>
#include <cmpth/wrap/basic_wrap_thread.hpp>

namespace cmpth {

struct mth_thread_policy
    : mth_base_policy
{
    using derived_type = mth_thread;
    
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
};

class mth_thread
    : public basic_wrap_thread<mth_thread_policy>
{
    using base = basic_wrap_thread<mth_thread_policy>;
    
public:
    using base::base;
};

} // namespace cmpth

