
#pragma once

#include <cmpth/wrap/abt/abt.hpp>
#include <cmpth/wrap/basic_wrap_thread.hpp>

namespace cmpth {

template <typename P>
struct abt_thread_policy
{
    using derived_type = basic_wrap_thread<abt_thread_policy>;
    
    using thread_ptr_type = ABT_thread;
    using thread_return_type = void;
    using thread_func_type = thread_return_type (*)(void*);

private:
    static ABT_thread_attr make_migratable_thread_attr() {
        ABT_thread_attr attr = ABT_THREAD_ATTR_NULL;
        abt_error::check_error(
            ABT_thread_attr_create(&attr)
        );
        // TODO: needs to free
        ABT_thread_attr_set_migratable(attr, true);
        return attr;
    }
    
public:
    static thread_ptr_type thread_create(thread_func_type f, void* const p) {
        ABT_thread ret = ABT_THREAD_NULL;
        const auto pool = P::get_pool();
        static auto attr = make_migratable_thread_attr();
        abt_error::check_error(
            ABT_thread_create(pool, f, p, attr, &ret)
        );
        return ret;
    }
    static void thread_join(const thread_ptr_type t) {
        abt_error::check_error(ABT_thread_join(t));
    }
    static thread_return_type make_thread_return_empty() { }
    
    using assert_policy_type = typename P::assert_policy_type;
    using log_aspect_type = typename P::log_aspect_type;
};

template <typename P>
using abt_thread = basic_wrap_thread<abt_thread_policy<P>>;

} // namespace cmpth

