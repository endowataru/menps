
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
struct sct_task_pool_base_policy
{
private:
    using lv4_itf_type = typename P::lv4_itf_type;
    
public:
    using element_type = typename lv4_itf_type::task_desc;
    
    template <typename Pool>
    static fdn::size_t get_pool_threshold(Pool& pool) {
        return P::get_task_pool_threshold(pool);
    }
    
    template <typename Node>
    static void destroy(Node* const n)
    {
        delete [] reinterpret_cast<fdn::byte*>(n->elem.stk_top);
    }
};

template <typename P>
class sct_task_pool
    : public P::lv4_itf_type::template pool_t<sct_task_pool_base_policy<P>>
{
    using lv4_itf_type = typename P::lv4_itf_type;
    using base = typename lv4_itf_type::template pool_t<sct_task_pool_base_policy<P>>;
    
    using worker_type           = typename lv4_itf_type::worker;
    using call_stack_type       = typename lv4_itf_type::call_stack;
    using task_ref_type         = typename lv4_itf_type::task_ref;
    using unique_task_ptr_type  = typename lv4_itf_type::unique_task_ptr;
    
    using typename base::node;
    
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
        sct_task_pool& self;
        node* operator() () {
            auto size = this->self.def_size_;
            auto alloc_p = new fdn::byte[size];
            void* cur_ptr = alloc_p + size;
            
            auto* const n =
                new (
                    fdn::align_call_stack(
                        alignof(node), sizeof(node),
                        cur_ptr, size
                    )
                ) node{};
            
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
    
private:
    fdn::size_t def_size_ = P::get_default_stack_size();
};

} // namespace cmpth

