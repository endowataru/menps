
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

#define MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET

template <typename P>
class alltoall_ptr_set
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
    void coll_make(rma_itf_type& /*rma*/, CollItf& coll, const lptr_type lptr, size_type size)
    // TODO: rma is not used in this function
    {
        const auto num_procs = coll.number_of_processes();
        
        #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
        num_procs_ = num_procs;
        num_elems_ = size;
        #endif
        
        this->lptr_ = lptr;
        const auto lptrs = mefdn::make_unique<lptr_type []>(num_procs);
        
        coll.allgather(&lptr, lptrs.get(), 1);
        
        this->rptrs_ =
            mefdn::make_unique<rptr_type []>(num_procs);
        
        for (process_id_type proc = 0; proc < num_procs; ++proc) {
            this->rptrs_[proc] = lptrs[proc];
        }
    }
    
    lptr_type local(const size_type idx) const noexcept {
        #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
        MEFDN_ASSERT(idx < num_elems_);
        #endif
        return this->lptr_ + idx;
    }
    
    rptr_type remote(const process_id_type proc, const size_type idx) {
        #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
        MEFDN_ASSERT(static_cast<size_type>(proc) < num_procs_);
        MEFDN_ASSERT(idx < num_elems_);
        #endif
        return this->rptrs_[proc] + idx;
    }
    
private:
    lptr_type lptr_;
    
    mefdn::unique_ptr<rptr_type []> rptrs_;
    
    #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
    size_type num_procs_ = 0;
    size_type num_elems_ = 0;
    #endif
};

} // namespace mecom2
} // namespace menps

