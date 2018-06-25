
#pragma once

#include <menps/mecom2/rma/rma_typed_itf.hpp>
#include <menps/mecom2/rma/rma_alloc_buf_copier.hpp>
#include <menps/medev2/ucx/uct/uct.hpp>
#include <cstring>
#ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
    #include <menps/mecom2/com/com_signal_state.hpp>
#endif

#ifndef MECOM2_USE_MEUCT
    #define MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
#endif
#if (!defined(MECOM2_USE_MEUCT) || defined(MEDEV2_AVOID_SWITCH_IN_SIGNAL))
    // If MECOM2_USE_MEUCT is off, explicit progress is required.
    // If MEDEV2_AVOID_SWITCH_IN_SIGNAL is on,
    // using uncond var in signal handler is prohibited.
    #define MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
#endif

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
    
    struct completion
        : uct_completion_t
    {
        completion()
            : uct_completion_t{ &on_complete, 1 }
        { }
        
        #ifdef MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
        mefdn::atomic<bool> flag{false};
        #else
        typename ult_itf_type::uncond_variable uv;
        #endif
        
    private:
        static void on_complete(
            uct_completion_t* const comp
        ,   const ucs_status_t      status
        ) {
            // Static downcast.
            auto& self = *static_cast<completion*>(comp);
            
            MEFDN_LOG_VERBOSE(
                "msg:Completion function was called.\t"
                "self:0x{:x}\t"
                "count:{}\t"
                "status:{}\t"
            ,   reinterpret_cast<mefdn::intptr_t>(&self)
            ,   self.count
            ,   status
            );
            
            #ifdef MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
            self.flag.store(true, mefdn::memory_order_release);
            #else
            self.uv.notify();
            #endif
        }
    };
    
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
        completion comp;
        
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
            ,   &comp
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
        
        completion comp;
        
        #if 1
        uct_iov iov = uct_iov();
        iov.buffer = dest_lptr.get();
        iov.length = num_bytes;
        iov.memh = dest_lptr.get_minfo()->mem.get();
        iov.stride = 0;
        iov.count = 1;
        
        ucs_status_t st = UCS_OK;
        {
            #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
            mefdn::lock_guard<spinlock_type> lk(info.lock);
            #endif
            
            st = info.ep.get_zcopy(
                &iov
            ,   1
            ,   src_rptr.get_addr()
            ,   info.rkey.get().rkey
            ,   &comp
            );
        }
        
        #else
        const auto st =
            info.ep.get_bcopy(
                reinterpret_cast<uct_unpack_callback_t>(&std::memcpy)
            ,   dest_lptr
            ,   num_bytes
            ,   src_rptr.get_addr()
            ,   info.rkey.get().rkey
            ,   &comp
            );
        #endif
        
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
        
        completion comp;
        
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
            ,   &comp                   // comp
            );
        }
        
        this->wait(info, st, &comp);
        
        self.unlock_rma(info);
    }
    
private:
    void wait(
        const rkey_info_type&   info
    ,   const ucs_status_t      st
    ,   completion* const       comp
    ) {
        auto& self = this->derived();
        
        if (st == UCS_OK) {
            return;
        }
        
        #ifdef MECOM2_UCT_RMA_ENABLE_EXPLICIT_PROGRESS
        while (true)
        {
            if (comp->flag.load(mefdn::memory_order_acquire)) {
                break;
            }
            
            #ifdef MEDEV2_AVOID_SWITCH_IN_SIGNAL
            if (! com_signal_state::is_in_signal()) {
                ult_itf_type::this_thread::yield();
            }
            #else
            ult_itf_type::this_thread::yield();
            #endif
            
            {
                #ifdef MECOM2_UCT_RMA_ENABLE_WORKER_LOCK
                mefdn::lock_guard<spinlock_type> lk(info.lock);
                #endif
                
                while (info.iface.progress() != 0) {
                    // Poll until there are completions
                }
            }
        }
        #else
        comp->uv.wait();
        #endif
    }
    
    void flush(
        const rkey_info_type&   info
    ,   const ucs_status_t      st
    ) {
        if (st == UCS_OK) {
            return;
        }
        
        completion comp;
        
        const auto ret =
            info.ep.flush(UCT_FLUSH_FLAG_LOCAL, &comp);
        
        this->wait(info, ret, &comp);
    }
};

} // namespace mecom2
} // namespace menps

