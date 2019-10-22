
#pragma once

#include <menps/medsm2/common.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace medsm2 {

template <typename P>
class id_allocator
{
    using com_itf_type = typename P::com_itf_type;
    using proc_id_type = typename com_itf_type::proc_id_type;
    
    using size_type = typename P::size_type;
    
    using atomic_int_type = typename P::atomic_int_type;
    
public:
    void coll_make(
        com_itf_type&   com
    ,   const size_type num_ids
    ) {
        auto& rma = com.get_rma();
        auto& coll = com.get_coll();
        
        this->num_ids_ = num_ids;
        
        this->buf_.coll_make(rma, coll, 1);
        if (com.this_proc_id() == 0) {
            *this->buf_.local(0) = 1;
        }
    }
    
    size_type allocate(
        com_itf_type&   com
    ) {
        auto& rma = com.get_rma();
        
        // TODO: Replace with remote fetch-and-add.
        
        const proc_id_type target_proc = 0;
        const auto target_rptr = buf_.remote(target_proc, 0);
        
        atomic_int_type expected = 0;
        {
            const auto expected_buf =
                rma.buf_read(target_proc, target_rptr, 1);
            
            expected = *expected_buf.get();
        }
        
        while (true)
        {
            const auto desired = expected + 1;
            
            const auto cas_result =
                rma.compare_and_swap(
                    target_proc // target_proc
                ,   target_rptr // target_rptr
                ,   expected    // expected
                ,   desired     // desired
                );
            
            if (cas_result == expected) {
                break;
            }
            
            expected = cas_result;
        }
        
        if (expected >= this->num_ids_) {
            throw std::bad_alloc{};
        }
        
        return static_cast<size_type>(expected);
    }
    
    void deallocate(
        com_itf_type&   /*com*/
    ,   const size_type /*id*/
    ) {
        // TODO
    }
    
private:
    size_type num_ids_ = 0;
    
    typename com_itf_type::template alltoall_buffer_t<atomic_int_type> buf_;
};

} // namespace medsm2
} // namespace menps

