
#pragma once

#include <cmpth/pool/basic_return_pool.hpp>

namespace cmpth {

template <typename UltItf, typename P2>
class basic_ext_return_pool;

template <typename UltItf, typename P2>
struct basic_ext_return_pool_policy
{
    using derived_type = basic_ext_return_pool<UltItf, P2>;
    using element_type = typename P2::element_type;
    using spinlock_type = typename UltItf::spinlock;
    
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& pool) {
        return P2::get_pool_threshold(pool);
    }
    
    using assert_policy_type = typename UltItf::assert_policy;
    using log_policy_type = typename UltItf::log_policy;
};

template <typename UltItf, typename P2>
class basic_ext_return_pool
    : public basic_return_pool<basic_ext_return_pool_policy<UltItf, P2>>
{
    using base = basic_return_pool<basic_ext_return_pool_policy<UltItf, P2>>;
    using ult_itf_type = UltItf;
    using element_type = typename P2::element_type;
    
public:
    using typename base::node;
    
    basic_ext_return_pool()
        : base{ult_itf_type::get_num_workers()}
    { }
    
    using base::allocate;
    
    template <typename AllocFunc>
    element_type* allocate(AllocFunc&& alloc_func) {
        const auto wk_num = ult_itf_type::get_worker_num();
        return base::allocate(wk_num, fdn::forward<AllocFunc>(alloc_func));
    }
    element_type* allocate() {
        return this->allocate(on_create{});
    }
    
private:
    struct on_create {
        node* operator() () {
            return new node;
        }
    };
    
public:
    void deallocate(element_type* e)
    {
        const auto wk_num = ult_itf_type::get_worker_num();
        base::deallocate(wk_num, e);
    }
    
public:
    static void destroy(node* const n)
    {
        delete n;
    }
};

} // namespace cmpth

