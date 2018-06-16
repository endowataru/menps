
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
    using proc_id_type = typename rma_itf_type::proc_id_type;
    
    using element_type = typename P::element_type;
    using size_type = typename P::size_type;
    
    using puptr_type =
        typename rma_itf_type::template public_ptr<element_type>;
    
    using rptr_type =
        typename rma_itf_type::template remote_ptr<element_type>;
    
public:
    template <typename CollItf>
    void coll_make(rma_itf_type& rma, CollItf& coll, const puptr_type puptr, size_type size)
    {
        const auto num_procs = coll.get_num_procs();
        
        #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
        num_procs_ = num_procs;
        num_elems_ = size;
        #endif
        
        this->puptr_ = puptr;
        
        const auto cur_sbuf_size = rma.serialized_size_in_bytes(this->puptr_);
        
        mefdn::size_t max_sbuf_size = 0;
        coll.allreduce_max(&cur_sbuf_size, &max_sbuf_size, 1);
        
        const auto cur_sbuf = mefdn::make_unique<mefdn::byte []>(max_sbuf_size);
        
        rma.serialize(this->puptr_, cur_sbuf.get());
        
        const auto all_sbuf = mefdn::make_unique<mefdn::byte []>(num_procs * max_sbuf_size);
        coll.allgather(cur_sbuf.get(), all_sbuf.get(), max_sbuf_size);
        
        this->rptrs_ = mefdn::make_unique<rptr_type []>(num_procs);
        
        for (proc_id_type proc = 0; proc < num_procs; ++proc) {
            this->rptrs_[proc] =
                rma.template deserialize<element_type>(
                    proc, &all_sbuf[proc * max_sbuf_size]);
        }
    }
    
    puptr_type local(const size_type idx) const noexcept {
        #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
        MEFDN_ASSERT(idx < num_elems_);
        #endif
        return this->puptr_ + idx;
    }
    
    rptr_type remote(const proc_id_type proc, const size_type idx) {
        #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
        MEFDN_ASSERT(static_cast<size_type>(proc) < num_procs_);
        MEFDN_ASSERT(idx < num_elems_);
        #endif
        return this->rptrs_[proc] + idx;
    }
    
private:
    puptr_type puptr_;
    
    mefdn::unique_ptr<rptr_type []> rptrs_;
    
    #ifdef MECOM2_ENABLE_DEBUG_ALLTOALL_PTR_SET
    size_type num_procs_ = 0;
    size_type num_elems_ = 0;
    #endif
};

template <typename Rma, typename Elem>
struct alltoall_ptr_set_policy
{
    using derived_type = alltoall_ptr_set<alltoall_ptr_set_policy>;
    
    using rma_itf_type = Rma;
    
    using proc_id_type = typename rma_itf_type::proc_id_type;
    using size_type = typename rma_itf_type::size_type;
    
    using element_type = Elem;
};

} // namespace mecom2
} // namespace menps

