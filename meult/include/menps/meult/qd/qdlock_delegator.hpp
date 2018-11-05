
#pragma once

#include <menps/meult/qd/basic_qdlock_pool.hpp>
#include <menps/meult/qd/uncond_qdlock_thread.hpp>
#include <menps/meult/qd/basic_qdlock_core.hpp>
#include <menps/meult/qd/basic_qdlock_delegator.hpp>
#include <menps/mefdn/container/intrusive_forward_list.hpp>

namespace menps {
namespace meult {

template <typename Func, typename UltItf>
struct qdlock_delegator_node
{
    menps::mefdn::atomic<qdlock_delegator_node*>    next;
    typename UltItf::uncond_variable*               uv;
    Func                                            func;
};

template <typename Func, typename UltItf>
struct qdlock_delegator_policy_base
{
    using policy_type = qdlock_delegator_policy_base<Func, UltItf>;
    
    using qdlock_pool_type = basic_qdlock_pool<policy_type>;
    using qdlock_core_type = basic_qdlock_core<policy_type>;
    using qdlock_node_type = qdlock_delegator_node<Func, UltItf>;
    using qdlock_thread_type = uncond_qdlock_thread<policy_type>;
    
    using atomic_node_ptr_type = menps::mefdn::atomic<qdlock_node_type*>;
    using atomic_bool_type = menps::mefdn::atomic<bool>;
    
    using qdlock_node_list_type = menps::mefdn::intrusive_forward_list<qdlock_node_type>;
    
    using ult_itf_type = UltItf;
};

template <typename Func, typename UltItf>
class qdlock_delegator;

template <typename Func, typename UltItf>
struct qdlock_delegator_policy
    : qdlock_delegator_policy_base<Func, UltItf>
{
    using derived_type = qdlock_delegator<Func, UltItf>;
};

template <typename Func, typename UltItf>
class qdlock_delegator
    : public basic_qdlock_delegator<qdlock_delegator_policy<Func, UltItf>>
{
    using P = qdlock_delegator_policy<Func, UltItf>;
    
public:
    using qdlock_pool_type = typename P::qdlock_pool_type;
    using qdlock_node_type = typename P::qdlock_node_type;
    
    explicit qdlock_delegator(qdlock_pool_type& pool)
        : pool_(pool)
    { }
    
    qdlock_pool_type& get_pool() const noexcept {
        return this->pool_;
    }
    
private:
    qdlock_pool_type& pool_;
};

} // namespace meult
} // namespace menps


// TODO: Refactoring

namespace menps {
namespace mefdn {

template <typename Func, typename UltItf>
struct intrusive_forward_list_traits<meult::qdlock_delegator_node<Func, UltItf>>
{
    typedef meult::qdlock_delegator_node<Func, UltItf>      node_base_type;
    typedef node_base_type*                                     node_pointer_type;
    
    using T = node_base_type;
     
    typedef T&                                  reference;
    typedef const T&                            const_reference;
    typedef T                                   value_type;
    typedef T*                                  pointer_type;
    typedef const T*                            const_pointer_type;
    
    static node_pointer_type get_next(const node_base_type& x) noexcept {
        return x.next.load(mefdn::memory_order_relaxed);
    }
    static void set_next(node_base_type& x, const node_pointer_type next) noexcept {
        x.next.store(next, mefdn::memory_order_relaxed);
    }
    
    static reference from_base(const node_pointer_type x) noexcept {
        MEFDN_ASSERT(x != nullptr);
        return static_cast<reference>(*x);
    }
    static node_pointer_type to_base(reference x) noexcept {
        return mefdn::addressof(x);
    }
};

} // namespace mefdn
} // namespace menps

