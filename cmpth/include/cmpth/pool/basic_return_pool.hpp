
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class basic_return_pool
{
    CMPTH_DEFINE_DERIVED(P)
    
    using element_type = typename P::element_type;
    using spinlock_type = typename P::spinlock_type;
    
    using size_type = fdn::size_t;
    
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
        pro_entry*  pros;
        node*       con_local;
        fdn::byte   pad1[
            fdn::calc_padding(sizeof(pro_entry*) + sizeof(node*))
        ];
        
        // Shared with other workers.
        spinlock_type   lock;
        node*           con_remote;
        fdn::byte       pad2[
            fdn::calc_padding(sizeof(spinlock_type) + sizeof(node*))
        ];
    };
    
public:
    explicit basic_return_pool(const size_type n_wks)
        : n_wks_(n_wks)
    {
        const auto min_n_pes = 
            fdn::roundup_divide<size_type>(CMPTH_CACHE_LINE_SIZE, sizeof(pro_entry));
        
        const auto n_pes_per_wk =
            fdn::roundup_divide(n_wks, min_n_pes) * min_n_pes;
        
        this->wes_ = fdn::make_oa_unique<wk_entry []>(n_wks);
        this->pes_ = fdn::make_oa_unique<pro_entry []>(n_wks * n_pes_per_wk);
        
        for (size_type i = 0; i < n_wks; ++i) {
            this->wes_[i].pros = &this->pes_[i * n_pes_per_wk];
        }
    }
    
    ~basic_return_pool()
    {
        const auto n_wks = this->n_wks_;
        
        for (size_type i = 0; i < n_wks; ++i) {
            auto& we = this->wes_[i];
            this->destroy_all(we.con_local);
            this->destroy_all(we.con_remote);
            
            for (size_type j = 0; j < n_wks; ++j) {
                auto pe = we.pros[j];
                this->destroy_all(pe.first);
            }
        }
    }
    
    basic_return_pool(const basic_return_pool&) = delete;
    basic_return_pool& operator = (const basic_return_pool&) = delete;
    
private:
    static void destroy_all(node* n) {
        while (n != nullptr) {
            const auto next = n->next;
            derived_type::destroy(n);
            n = next;
        }
    }
    
public:
    template <typename AllocFunc>
    element_type* allocate(size_type cur_wk_num, AllocFunc&& alloc_func)
    {
        //const auto cur_wk_num = ult_itf_type::get_worker_num();
        auto& cur_we = this->wes_[cur_wk_num];
        
        // Try to consume local entries.
        auto ret = cur_we.con_local;
        
        if (CMPTH_LIKELY(ret != nullptr)) {
            // Consumed one entry.
            cur_we.con_local = ret->next;
            
            // Placement-new. No overhead for PODs.
            new (&ret->elem) element_type;
            ret->wk_num = cur_wk_num;
            
            return &ret->elem;
        }
        
        {
            // Try to consume remote entries.
            fdn::unique_lock<spinlock_type> lk(cur_we.lock);
            ret = cur_we.con_remote;
            
            if (CMPTH_LIKELY(ret != nullptr)) {
                cur_we.con_remote = nullptr;
                lk.unlock();
                
                cur_we.con_local = ret->next;
                
                // Placement-new. No overhead for PODs.
                new (&ret->elem) element_type;
                ret->wk_num = cur_wk_num;
                
                return &ret->elem;
            }
        }
        
        ret = fdn::forward<AllocFunc>(alloc_func)();
        ret->wk_num = cur_wk_num;
        
        return &ret->elem;
    }
    
    #if 0
    element_type* allocate(size_type cur_wk_num)
    {
        return this->allocate(cur_wk_num, [] { return new node; });
    }
    #endif
    
    void deallocate(size_type cur_wk_num, element_type* const e)
    {
        // Destruct the element. No overhead for PODs.
        e->~element_type();
        
        const auto n = fdn::get_container_of(e, &node::elem);
        const auto alloc_wk_num = n->wk_num;
        
        //const auto cur_wk_num = ult_itf_type::get_worker_num();
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
                // Store remote entries to the local list.
                if (pe_num == 0) {
                    CMPTH_P_ASSERT(P, pe.first == nullptr);
                    CMPTH_P_ASSERT(P, pe.last == nullptr);
                    pe.last = n;
                }
                else {
                    CMPTH_P_ASSERT(P, pe.first != nullptr);
                    CMPTH_P_ASSERT(P, pe.last != nullptr);
                }
                n->next = pe.first;
                pe.first = n;
                pe.num = pe_num + 1;
            }
            else {
                // Put remote entries back to the remote list.
                auto& alloc_we = this->wes_[alloc_wk_num];
                
                const auto pe_first = pe.first;
                const auto pe_last = pe.last;
                CMPTH_P_ASSERT(P, pe_first != nullptr);
                CMPTH_P_ASSERT(P, pe_last != nullptr);
                
                {
                    fdn::lock_guard<spinlock_type> lk(alloc_we.lock);
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
        return fdn::get_container_of(e, &node::elem);
    }
    
private:
    size_type                           n_wks_ = 0;
    fdn::oa_unique_ptr<wk_entry []>     wes_;
    fdn::oa_unique_ptr<pro_entry []>    pes_;
};

} // namespace cmpth

