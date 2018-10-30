
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/memory/get_container_of.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_tls_pool
{
    using element_type = typename P::element_type;
    using ult_itf_type = typename P::ult_itf_type;
    using size_type = typename P::size_type;
    
    struct node {
        node*           next;
        element_type    elem;
    };
    
public:
    basic_tls_pool()
        : ls_(
            mefdn::make_unique<node* []>(
                ult_itf_type::get_num_workers()
            )
        )
    { }
    
    ~basic_tls_pool()
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
        }
        else {
            n = l;
            l = n->next;
            
            // Placement-new.
            new (&n->elem) element_type();
        }
        
        return &n->elem;
    }
    
    void deallocate(element_type* /*const (TODO)*/ e)
    {
        // Destruct the element.
        e->~element_type();
        
        const auto n = mefdn::get_container_of(e, &node::elem);
        
        auto& l = this->get_local();
        
        n->next = l;
        l = n;
    }
    
private:
    node*& get_local() const noexcept {
        const auto i = ult_itf_type::get_worker_num();
        return this->ls_[i];
    }
    
    mefdn::unique_ptr<node* []> ls_;
};

} // namespace meult
} // namespace menps

