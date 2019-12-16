
#pragma once

#include <cmpth/pool/basic_return_pool.hpp>

namespace cmpth {

template <typename P>
class basic_sct_return_memory_pool;

template <typename P>
struct basic_sct_return_memory_pool_policy
{
    using derived_type = basic_sct_return_memory_pool<P>;
    using element_type = typename P::element_type;
    using spinlock_type = typename P::spinlock_type;
    
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& pool) {
        return P::get_pool_threshold(pool);
    }
    
    using assert_policy_type = typename P::assert_policy_type;
    using log_aspect_type = typename P::log_aspect_type;
};

template <typename P>
class basic_sct_return_memory_pool
    : public basic_return_pool<basic_sct_return_memory_pool_policy<P>>
{
    using base = basic_return_pool<basic_sct_return_memory_pool_policy<P>>;

    using element_type = typename P::element_type;
    using worker_type = typename P::worker_type;

public:
    using typename base::node;

    using base::base;

    basic_sct_return_memory_pool()
        : base{P::get_num_workers()}
    { }

    using base::allocate;

    template <typename AllocFunc>
    element_type* allocate(AllocFunc&& alloc_func) {
        auto& wk = worker_type::get_cur_worker();
        const auto wk_num = wk.get_worker_num();
        return base::allocate(wk_num, fdn::forward<AllocFunc>(alloc_func));
    }
    element_type* allocate() {
        return this->allocate(on_create{ *this });
    }
    
private:
    struct on_create {
        basic_sct_return_memory_pool& self;
        node* operator() () {
            return P::template create<node>(self);
        }
    };

public:
    using base::deallocate;
    
    void deallocate(element_type* e) {
        auto& wk = worker_type::get_cur_worker();
        const auto wk_num = wk.get_worker_num();
        base::deallocate(wk_num, e);
    }
    static void destroy(node* const n) {
        P::destroy(n);
    }
};

} // namespace cmpth

