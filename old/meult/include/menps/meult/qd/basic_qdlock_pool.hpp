
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_qdlock_pool
{
    using qdlock_node_type = typename P::qdlock_node_type;
    using qdlock_node_list_type = typename P::qdlock_node_list_type;
        //mefdn::intrusive_forward_list<qdlock_node_type>
    
    using ult_itf_type = typename P::ult_itf_type;
    
public:
    basic_qdlock_pool()
        : ls_(
            mefdn::make_unique<qdlock_node_list_type []>(
                ult_itf_type::get_num_workers()
            )
        )
    { }
    
    qdlock_node_type* allocate()
    {
        auto& l = this->get_local_list();
        
        if (MEFDN_UNLIKELY(l.empty())) {
            return new qdlock_node_type;
        }
        else {
            auto& n = l.front();
            l.pop_front();
            
            return &n;
        }
    }
    
    void deallocate(qdlock_node_type* /*const (TODO)*/ p)
    {
        auto& l = this->get_local_list();
        l.push_front(*p);
    }
    
private:
    qdlock_node_list_type& get_local_list() const noexcept {
        return this->ls_[ult_itf_type::get_worker_num()];
    }
    
    mefdn::unique_ptr<qdlock_node_list_type []> ls_;
};

} // namespace meult
} // namespace menps

