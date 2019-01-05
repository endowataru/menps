
#pragma once

#include <menps/mecom2/rma/rma_typed_itf.hpp>
#include <menps/mecom2/rma/rma_alloc_buf_copier.hpp>
#include <menps/medev2/ucx/uct/uct.hpp>
#include <cstring>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_uct_rma
    : public rma_typed_itf<P>
    , public rma_alloc_buf_copier<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ult_itf_type = typename P::ult_itf_type;
    using spinlock_type = typename ult_itf_type::spinlock;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using local_ptr_t = typename P::template local_ptr<T>;
    
    using rkey_info_type = typename P::rkey_info_type;
    
    using completion_type = typename P::completion_type;
    
    struct write_arg {
        const void* src_ptr;
        size_type   num_bytes;
    };
    
    static mefdn::size_t on_write_pack(void* const dest, void* const arg_void)
    {
        auto& arg = *static_cast<const write_arg*>(arg_void);
        std::memcpy(dest, arg.src_ptr, arg.num_bytes);
        return arg.num_bytes;
    }
    
public:
    void untyped_write(
        const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        auto info = self.lock_rma(dest_proc, dest_rptr);
        
        #if 1
        completion_type comp;
        
        uct_iov iov = uct_iov();
        iov.buffer = const_cast<void*>(src_lptr.get());
        iov.length = num_bytes;
        iov.memh = src_lptr.get_minfo()->mem.get();
        iov.stride = 0;
        iov.count = 1;
        
        ucs_status_t st = UCS_OK;
        {
            #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
            mefdn::lock_guard<spinlock_type> lk(info.lock);
            #endif
            
            st = info.ep.put_zcopy(
                &iov
            ,   1
            ,   dest_rptr.get_addr()
            ,   info.rkey.get().rkey
            ,   comp.get_ptr()
            );
        }
        
        this->wait(info, st, &comp);
        
        #else
        write_arg arg{ src_lptr, num_bytes };
        
        // Note: Return value (= length) is ignored.
        info.ep.put_bcopy(
            &on_write_pack
        ,   &arg
        ,   dest_rptr.get_addr()
        ,   info.rkey.get().rkey
        );
        
        this->flush(info, UCS_INPROGRESS);
        #endif
        
        self.unlock_rma(info);
    }
    
    void untyped_read(
        const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        auto info = self.lock_rma(src_proc, src_rptr);
        
        completion_type comp;
        
        ucs_status_t st = UCS_OK;
        
        // TODO: Read threshold from UCT config
        if (num_bytes > MECOM2_RMA_UCT_GET_ZCOPY_SIZE)
        {
            uct_iov iov = uct_iov();
            iov.buffer = dest_lptr.get();
            iov.length = num_bytes;
            iov.memh = dest_lptr.get_minfo()->mem.get();
            iov.stride = 0;
            iov.count = 1;
            
            
            {
                #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
                mefdn::lock_guard<spinlock_type> lk(info.lock);
                #endif
                
                st = info.ep.get_zcopy(
                    &iov
                ,   1
                ,   src_rptr.get_addr()
                ,   info.rkey.get().rkey
                ,   comp.get_ptr()
                );
            }
        }
        else
        {
            st = info.ep.get_bcopy(
                reinterpret_cast<uct_unpack_callback_t>(&std::memcpy)
            ,   dest_lptr
            ,   num_bytes
            ,   src_rptr.get_addr()
            ,   info.rkey.get().rkey
            ,   comp.get_ptr()
            );
        }
        
        this->wait(info, st, &comp);
        //this->flush(info, UCS_INPROGRESS);
        
        self.unlock_rma(info);
    }
    
    // TODO: Currently, Atomic operation don't require local buffer registration.
    template <typename T>
    void compare_and_swap_b(
        const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const T* const              expected_lptr
    ,   const T* const              desired_lptr
    ,   T* const                    result_lptr
    ) {
        auto& self = this->derived();
        auto info = self.lock_rma(target_proc, target_rptr);
        
        completion_type comp;
        
        ucs_status_t st = UCS_OK;
        {
            #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
            mefdn::lock_guard<spinlock_type> lk(info.lock);
            #endif
        
            st = info.ep.atomic_cswap64(
                *expected_lptr          // compare
            ,   *desired_lptr           // swap
            ,   target_rptr.get_addr()  // remote_addr
            ,   info.rkey.get().rkey    // rkey
            ,   result_lptr             // result
            ,   comp.get_ptr()          // comp
            );
        }
        
        this->wait(info, st, &comp);
        
        self.unlock_rma(info);
    }
    
private:
    void wait(
        const rkey_info_type&   info MEFDN_MAYBE_UNUSED
    ,   const ucs_status_t      st
    ,   completion_type* const  comp
    ) {
        if (st == UCS_OK) {
            return;
        }
        
        comp->wait(info);
    }
    
    void flush(
        const rkey_info_type&   info
    ,   const ucs_status_t      st
    ) {
        if (st == UCS_OK) {
            return;
        }
        
        completion_type comp;
        
        const auto ret =
            info.ep.flush(UCT_FLUSH_FLAG_LOCAL, &comp);
        
        this->wait(info, ret, &comp);
    }
};

} // namespace mecom2
} // namespace menps

