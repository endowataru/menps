
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

template <typename P>
class smth_task_pool
    : protected P::pool_base_type
{
    using base = typename P::pool_base_type;
    
    using worker_type = typename P::worker_type;
    using call_stack_type = typename P::call_stack_type;
    
    using task_ref_type = typename P::task_ref_type;
    using unique_task_ptr_type = typename P::unique_task_ptr_type;
    
    using typename base::node;
    using node_type = node;
    
public:
    using base::base;
    
    call_stack_type allocate(worker_type& wk)
    {
        const auto wk_num = wk.get_worker_num();
        
        const auto desc = base::allocate(wk_num, on_create{ *this });
        
        desc->finished.store(false, fdn::memory_order_relaxed);
        
        return call_stack_type{ unique_task_ptr_type{ desc } };
    }
    
private:
    struct on_create {
        smth_task_pool& self;
        node* operator() () {
            auto size = this->self.def_size_;
            auto alloc_p = new fdn::byte[size];
            void* cur_ptr = alloc_p + size;
            
            auto* const n =
                new (
                    fdn::align_call_stack(
                        alignof(node_type), sizeof(node_type),
                        cur_ptr, size
                    )
                ) node_type{};
            
            auto desc = &n->elem;
            desc->stk_top = alloc_p;
            desc->stk_bottom = n;
            return n;
        }
    };
    
public:
    void deallocate(worker_type& wk, task_ref_type tk)
    {
        const auto wk_num = wk.get_worker_num();
        base::deallocate(wk_num, tk.get_task_desc());
    }
    
public:
    static void destroy(node* const n)
    {
        delete [] reinterpret_cast<fdn::byte*>(n->elem.stk_top);
    }
    
private:
    fdn::size_t def_size_ = P::get_default_stack_size();
};

} // namespace cmpth

