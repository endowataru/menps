
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/memory/get_container_of.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_numbered_pool
{
    using base_pool_type = typename P::base_pool_type;
    using node_type = typename base_pool_type::node;
    
    using element_type = typename P::element_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    
    using size_type = typename P::size_type;
    
public:
    explicit basic_numbered_pool(const size_type max_num_elems)
        : max_num_elems_(max_num_elems)
        , nodes_(
            mefdn::make_unique<node_type []>(max_num_elems)
        )
    { }
    
    ~basic_numbered_pool() = default;
    
private:
    struct on_alloc
    {
        basic_numbered_pool& self;
        
        node_type* operator() () const {
            mefdn::lock_guard<spinlock_type> lk(self.lock_);
            const auto num = self.num_++;
            if (MEFDN_UNLIKELY(num >= self.max_num_elems_)) {
                throw std::bad_alloc();
            }
            return &self.nodes_[num];
        }
    };
    
public:
    element_type* allocate()
    {
        return this->pool_.allocate(on_alloc{ *this });
    }
    void deallocate(element_type* const e)
    {
        this->pool_.deallocate(e);
    }
    
    size_type to_number(element_type* const e) const noexcept
    {
        return base_pool_type::to_node(e) - this->nodes_.get();
    }
    
    element_type* to_pointer(const size_type num) const noexcept
    {
        return base_pool_type::to_elem(&this->nodes_[num]);
    }
    
private:
    const size_type max_num_elems_ = 0;
    
    mefdn::unique_ptr<node_type []>  nodes_;
    // Note: nodes_ must be deallocated at last.
    base_pool_type  pool_;
    
    spinlock_type   lock_;
    size_type       num_ = 0;
};

} // namespace meult
} // namespace menps

