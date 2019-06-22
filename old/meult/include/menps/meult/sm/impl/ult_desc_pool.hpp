
#pragma once

#include "ult_desc.hpp"
#include "ult_ptr_ref.hpp"
#include <menps/mefdn/container/intrusive_forward_list.hpp>

namespace menps {
namespace meult {
namespace sm {

struct ult_desc_node
    : ult_desc
    , mefdn::intrusive_forward_list_node_base
{ };

class ult_desc_pool
{
public:
    ult_desc_pool() { }
    
    ult_ptr_ref allocate_ult()
    {
        if (MEFDN_LIKELY(!l_.empty()))
        {
            auto& desc = l_.front();
            l_.pop_front();
            
            initialize_desc(&desc);
            
            ult_id id{ &desc };
            return ult_ptr_ref(id);
        }
        
        return create_ult();
    }
    
    void deallocate_ult(ult_ptr_ref&& th)
    {
        auto& desc = th.get_desc();
        
        auto& desc_node = static_cast<ult_desc_node&>(desc);
        
        l_.push_front(desc_node);
    }
    
private:
    static void initialize_desc(ult_desc* const desc)
    {
        desc->state = ult_state::ready;
        desc->detached = false;
        desc->joiner = static_cast<ult_desc_node*>(make_invalid_ult_id().ptr);
    }
    
    static ult_ptr_ref create_ult();
    
    static void destory_ult(ult_ptr_ref&& th);
    
    mefdn::intrusive_forward_list<ult_desc_node> l_;
};

} // namespace sm
} // namespace meult
} // namespace menps

