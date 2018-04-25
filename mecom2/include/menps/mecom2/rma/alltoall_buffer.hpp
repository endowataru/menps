
#pragma once

#include <menps/mecom2/rma/alltoall_ptr_set.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class alltoall_buffer
{
    MEFDN_DEFINE_DERIVED(P)
    
    using rma_itf_type = typename P::rma_itf_type;
    using proc_id_type = typename rma_itf_type::proc_id_type;
    
    using element_type = typename P::element_type;
    using size_type = typename P::size_type;
    
    using lptr_type =
        typename rma_itf_type::template local_ptr<element_type>;
    
    using rptr_type =
        typename rma_itf_type::template remote_ptr<element_type>;
    
public:
    template <typename CollItf>
    void coll_make(rma_itf_type& rma, CollItf& coll, const size_type num_elems_per_proc)
    {
        this->lptr_ =
            rma.template make_unique<element_type []>(num_elems_per_proc);
        
        this->ptrs_.coll_make(rma, coll, this->lptr_.get(), num_elems_per_proc);
    }
    
    lptr_type local(const size_type idx) const noexcept {
        return this->ptrs_.local(idx);
    }
    
    rptr_type remote(const proc_id_type proc, const size_type idx) {
        return this->ptrs_.remote(proc, idx);
    }
    
private:
    alltoall_ptr_set<P> ptrs_;
    
    typename rma_itf_type::
        template unique_local_ptr<element_type []> lptr_;
};

} // namespace mecom2
} // namespace menps

