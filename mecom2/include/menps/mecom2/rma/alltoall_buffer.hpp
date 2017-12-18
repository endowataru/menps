
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class alltoall_buffer
{
    MEFDN_DEFINE_DERIVED(P)
    
    using rma_itf_type = typename P::rma_itf_type;
    using process_id_type = typename rma_itf_type::process_id_type;
    
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
        const auto num_procs = coll.number_of_processes();
        
        this->lptr_ =
            rma.template make_unique<element_type []>(num_elems_per_proc);
        
        const auto lptr = this->lptr_.get();
        const auto lptrs = mefdn::make_unique<lptr_type []>(num_procs);
        
        coll.allgather(&lptr, lptrs.get(), 1);
        
        this->rptrs_ =
            mefdn::make_unique<rptr_type []>(num_procs);
        
        for (process_id_type proc = 0; proc < num_procs; ++proc) {
            this->rptrs_[proc] = lptrs[proc];
        }
    }
    
    lptr_type local(const size_type idx) {
        return this->lptr_.get() + idx;
    }
    
    rptr_type remote(const process_id_type proc, const size_type idx) {
        return this->rptrs_[proc] + idx;
    }
    
private:
    typename rma_itf_type::
        template unique_local_ptr<element_type []> lptr_;
    
    mefdn::unique_ptr<rptr_type []> rptrs_;
};

} // namespace mecom2
} // namespace menps

