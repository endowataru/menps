
#pragma once

#include <cmpth/pool/basic_return_pool.hpp>

namespace cmpth {

template <typename UltItf, typename P2>
class basic_numbered_pool;

template <typename UltItf, typename P2>
struct basic_numbered_pool_policy
{
    using derived_type = basic_numbered_pool<UltItf, P2>;
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
class basic_numbered_pool;

template <typename UltItf, typename P2>
class basic_numbered_pool_base
{
    using size_type = fdn::size_t;
    using derived_type = basic_numbered_pool<UltItf, P2>;
    using node = typename basic_return_pool<basic_numbered_pool_policy<UltItf, P2>>::node;
    
protected:
    explicit basic_numbered_pool_base(const size_type max_num_elems)
        : max_num_elems_{max_num_elems}
        , nodes_{
            fdn::make_unique<node []>(max_num_elems)
        }
    { }
    
    const size_type max_num_elems_ = 0;
    
    fdn::unique_ptr<node []>  nodes_;
    // Note: nodes_ must be deallocated at last.
};

template <typename UltItf, typename P2>
class basic_numbered_pool
    : private basic_numbered_pool_base<UltItf, P2>
    , public basic_return_pool<basic_numbered_pool_policy<UltItf, P2>>
{
    using base_nodes = basic_numbered_pool_base<UltItf, P2>;
    using base_pool = basic_return_pool<basic_numbered_pool_policy<UltItf, P2>>;
    using ult_itf_type = UltItf;
    using element_type = typename P2::element_type;
    
    using spinlock_type = typename ult_itf_type::spinlock;
    
    using size_type = fdn::size_t;
    
public:
    using typename base_pool::node;
    
    explicit basic_numbered_pool(const size_type max_num_elems)
        : base_nodes{max_num_elems}
        , base_pool{ult_itf_type::get_num_workers()}
    { }
    
    ~basic_numbered_pool() = default;
    
public:
    element_type* allocate()
    {
        const auto wk_num = ult_itf_type::get_worker_num();
        return base_pool::allocate(wk_num, on_alloc{ *this });
    }
    
private:
    struct on_alloc
    {
        basic_numbered_pool& self;
        
        node* operator() () const {
            fdn::lock_guard<spinlock_type> lk{self.lock_};
            const auto num = self.num_++;
            if (CMPTH_UNLIKELY(num >= self.max_num_elems_)) {
                throw std::bad_alloc();
            }
            return &self.nodes_[num];
        }
    };
    
public:
    void deallocate(element_type* const e)
    {
        const auto wk_num = ult_itf_type::get_worker_num();
        base_pool::deallocate(wk_num, e);
    }
    
    size_type to_number(element_type* const e) const noexcept
    {
        return base_pool::to_node(e) - this->nodes_.get();
    }
    
    element_type* to_pointer(const size_type num) const noexcept
    {
        return base_pool::to_elem(&this->nodes_[num]);
    }
    
    static void destroy(node* const /*n*/)
    {
        // Note: No need to call "delete" here for basic_numbered_pool.
    }
    
private:
    spinlock_type   lock_;
    size_type       num_ = 0;
};

} // namespace cmpth

