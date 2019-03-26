
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

template <typename P>
class smth_mcs_node_pool
    : protected P::pool_base_type
{
    using base = typename P::pool_base_type;
    
    using worker_type = typename P::worker_type;
    using element_type = typename P::element_type;
    
    using typename base::node;
    using node_type = node;
    
public:
    using base::base;
    
    element_type* allocate(worker_type& wk)
    {
        const auto wk_num = wk.get_worker_num();
        
        return base::allocate(wk_num, on_create{});
    }
    
private:
    struct on_create {
        node* operator() () {
            return new node;
        }
    };
    
public:
    void deallocate(worker_type& wk, element_type* e)
    {
        const auto wk_num = wk.get_worker_num();
        base::deallocate(wk_num, e);
    }
    
public:
    static void destroy(node* const n)
    {
        delete n;
    }
};

} // namespace cmpth


