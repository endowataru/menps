
#pragma once

#include <menps/meuct/common.hpp>
#include <menps/mefdn/container/intrusive_forward_list.hpp>

namespace menps {
namespace meuct {

template <typename P>
class proxy_completion_pool
{
    using proxy_completion_type = typename P::proxy_completion_type;
    
    struct completion_node
        : mefdn::intrusive_forward_list_node_base
        , proxy_completion_type
    { };
    
public:
    proxy_completion_type* allocate()
    {
        if (MEFDN_UNLIKELY(this->l_.empty())) {
            return new completion_node;
        }
        
        const auto ret = &this->l_.front();
        this->l_.pop_front();
        
        return ret;
    }
    
    void deallocate(proxy_completion_type* const pc)
    {
        // Static downcast.
        const auto node = static_cast<completion_node*>(pc);
        
        this->l_.push_front(*node);
    }
    
private:
    mefdn::intrusive_forward_list<completion_node> l_;
};

} // namespace meuct
} // namespace menps
