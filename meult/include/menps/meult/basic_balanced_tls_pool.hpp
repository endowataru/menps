
#pragma once

#include <menps/meult/common.hpp>
#include <menps/mefdn/memory/get_container_of.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/mutex.hpp>

namespace menps {
namespace meult {

template <typename P>
class basic_balanced_tls_pool
{
    using element_type = typename P::element_type;
    using ult_itf_type = typename P::ult_itf_type;
    using size_type = typename P::size_type;
    
    using spinlock_type = typename ult_itf_type::spinlock;
    
public:
    struct node {
        node*           next;
        size_type       wk_num;
        element_type    elem;
    };
    
private:
    struct pro_entry {
        node*           first;
        node*           last;
        size_type       num;
    };
    
    struct wk_entry {
        // Owned by the worker thread.
        pro_entry*      pros;
        node*           con_local;
        mefdn::byte     pad1[
            MEFDN_CACHE_LINE_SIZE - sizeof(pro_entry*) - sizeof(node*)
        ];
        
        // Shared with other workers.
        spinlock_type   lock;
        node*           con_remote;
        mefdn::byte     pad2[
            MEFDN_CACHE_LINE_SIZE - sizeof(spinlock_type) - sizeof(node*)
        ];
    };
    
public:
    basic_balanced_tls_pool()
    {
        const auto n_wks = ult_itf_type::get_num_workers();
        
        const auto min_n_pes = 
            mefdn::roundup_divide<size_type>(MEFDN_CACHE_LINE_SIZE, sizeof(pro_entry));
        
        const auto n_pes_per_wk =
            mefdn::roundup_divide(n_wks, min_n_pes) * min_n_pes;
        
        this->wes_ = mefdn::make_unique<wk_entry []>(n_wks);
        this->pes_ = mefdn::make_unique<pro_entry []>(n_wks * n_pes_per_wk);
        
        for (size_type i = 0; i < n_wks; ++i) {
            this->wes_[i].pros = &this->pes_[i * n_pes_per_wk];
        }
    }
    
    ~basic_balanced_tls_pool()
    {
        const auto n_wks = ult_itf_type::get_num_workers();
        
        for (size_type i = 0; i < n_wks; ++i) {
            auto& we = this->wes_[i];
            deallocate_all(we.con_local);
            deallocate_all(we.con_remote);
            
            for (size_type j = 0; j < n_wks; ++j) {
                auto pe = we.pros[j];
                deallocate_all(pe.first);
            }
        }
    }
    
    basic_balanced_tls_pool(const basic_balanced_tls_pool&) = delete;
    basic_balanced_tls_pool& operator = (const basic_balanced_tls_pool&) = delete;
    
private:
    static void deallocate_all(node* n) {
        while (n != nullptr) {
            const auto next = n->next;
            P::deallocate(n);
            n = next;
        }
    }
    
public:
    
    template <typename AllocFunc>
    //MEFDN_NOINLINE
    element_type* allocate(AllocFunc alloc_func)
    {
        const auto cur_wk_num = ult_itf_type::get_worker_num();
        auto& cur_we = this->wes_[cur_wk_num];
        
        // Try to consume local entries.
        auto ret = cur_we.con_local;
        
        if (MEFDN_LIKELY(ret != nullptr)) {
            // Consumed one entry.
            cur_we.con_local = ret->next;
            
            // Placement-new. No overhead for PODs.
            new (&ret->elem) element_type;
            ret->wk_num = cur_wk_num;
            
            return &ret->elem;
        }
        
        {
            // Try to consume remote entries.
            mefdn::unique_lock<spinlock_type> lk(cur_we.lock);
            ret = cur_we.con_remote;
            
            if (MEFDN_LIKELY(ret != nullptr)) {
                cur_we.con_remote = nullptr;
                lk.unlock();
                
                cur_we.con_local = ret->next;
                
                // Placement-new. No overhead for PODs.
                new (&ret->elem) element_type;
                ret->wk_num = cur_wk_num;
                
                return &ret->elem;
            }
        }
        
        ret = alloc_func();
        ret->wk_num = cur_wk_num;
        
        return &ret->elem;
    }
    
    element_type* allocate()
    {
        return this->allocate([] { return new node; });
    }
    
    //MEFDN_NOINLINE
    void deallocate(element_type* const e)
    {
        // Destruct the element. No overhead for PODs.
        e->~element_type();
        
        const auto n = mefdn::get_container_of(e, &node::elem);
        const auto alloc_wk_num = n->wk_num;
        
        const auto cur_wk_num = ult_itf_type::get_worker_num();
        auto& we = this->wes_[cur_wk_num];
        
        if (alloc_wk_num == cur_wk_num) {
            // Deallocate to the local pool.
            n->next = we.con_local;
            we.con_local = n;
        }
        else {
            auto& pe = we.pros[alloc_wk_num];
            const auto pe_num = pe.num;
            if (pe_num < P::get_pool_threshold(*this)) {
                if (pe_num == 0) {
                    MEFDN_ASSERT(pe.first == nullptr);
                    MEFDN_ASSERT(pe.last == nullptr);
                    pe.last = n;
                }
                else {
                    MEFDN_ASSERT(pe.first != nullptr);
                    MEFDN_ASSERT(pe.last != nullptr);
                }
                n->next = pe.first;
                pe.first = n;
                pe.num = pe_num + 1;
            }
            else {
                auto& alloc_we = this->wes_[alloc_wk_num];
                
                const auto pe_first = pe.first;
                const auto pe_last = pe.last;
                MEFDN_ASSERT(pe_first != nullptr);
                MEFDN_ASSERT(pe_last != nullptr);
                
                {
                    mefdn::lock_guard<spinlock_type> lk(alloc_we.lock);
                    pe_last->next = alloc_we.con_remote;
                    alloc_we.con_remote = pe_first;
                }
                
                pe.first = n;
                pe.last = n;
                pe.num = 1;
                n->next = nullptr;
            }
        }
    }
    
    static element_type* to_elem(node* const n) noexcept {
        return &n->elem;
    }
    static node* to_node(element_type* const e) noexcept {
        return mefdn::get_container_of(e, &node::elem);
    }
    
private:
    mefdn::unique_ptr<wk_entry []>  wes_;
    mefdn::unique_ptr<pro_entry []> pes_;
};

} // namespace meult
} // namespace menps

