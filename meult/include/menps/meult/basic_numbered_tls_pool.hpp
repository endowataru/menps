
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/memory/get_container_of.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_numbered_tls_pool
{
    using element_type = typename P::element_type;
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    using size_type = typename P::size_type;
    
    struct node {
        node*           next;
        size_type       num;
        element_type    elem;
    };
    
public:
    explicit basic_numbered_tls_pool(size_type max_num_elems)
        : max_num_elems_(max_num_elems)
        , num_(0)
        , ls_(
            mefdn::make_unique<node* []>(
                ult_itf_type::get_num_workers()
            )
        )
        , nodes_(
            mefdn::make_unique<node* []>(
                max_num_elems
            )
        )
    { }
    
    ~basic_numbered_tls_pool()
    {
        const auto nwks = ult_itf_type::get_num_workers();
        for (size_type i = 0; i < nwks; ++i) {
            while (this->ls_[i] != nullptr) {
                const auto p = this->ls_[i];
                this->ls_[i] = p->next;
                delete p;
            }
        }
    }
    
    element_type* allocate()
    {
        auto& l = this->get_local();
        node* n = nullptr;
        
        if (MEFDN_UNLIKELY(l == nullptr)) {
            // Value-initialization.
            n = new node();
            
            size_type num = 0;
            {
                mefdn::lock_guard<spinlock_type> lk(this->mtx_);
                num = this->num_++;
                MEFDN_ASSERT(num < this->max_num_elems_);
            }
            
            n->num = num; // for pointer-to-number
            this->nodes_[num] = n; // for number-to-pointer
        }
        else {
            n = l;
            l = n->next;
            
            // Placement-new.
            new (&n->elem) element_type();
        }
        
        return &n->elem;
    }
    
    void deallocate(element_type* const e)
    {
        // Destruct the element.
        e->~element_type();
        
        const auto n = this->to_node(e);
        
        auto& l = this->get_local();
        
        n->next = l;
        l = n;
    }
    
    size_type to_number(element_type* e)
    {
        const auto n = this->to_node(e);
        MEFDN_ASSERT(n->num < this->max_num_elems_);
        return n->num;
    }
    
    element_type* to_pointer(const size_type num)
    {
        const auto n = this->nodes_[num];
        MEFDN_ASSERT(n != nullptr);
        return &n->elem;
    }
    
private:
    node*& get_local() const noexcept {
        const auto i = ult_itf_type::get_worker_num();
        return this->ls_[i];
    }
    
    node* to_node(element_type* const e)
    {
        return mefdn::get_container_of(e, &node::elem);
    }
   
    const size_type max_num_elems_;
   
    spinlock_type mtx_;
    size_type num_;
    
    mefdn::unique_ptr<node* []> ls_;
    
    // Converts a node number to the node pointer.
    mefdn::unique_ptr<node* []> nodes_;
};

} // namespace meult
} // namespace menps

