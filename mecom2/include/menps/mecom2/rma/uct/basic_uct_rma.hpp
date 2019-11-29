
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
    
    using request_object_type = typename P::request_object_type;

public:
    using unique_request_type = typename P::unique_request_type;

private:
    unique_request_type allocate_unique_request() {
        return fdn::make_unique<request_object_type>();
    }
    
public:
    void wait(unique_request_type req)
    {
        auto& self = this->derived();
        MEFDN_ASSERT(req);
        req->wait(self);
    }

    unique_request_type untyped_write_nb(
        const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        auto req = self.allocate_unique_request();
        self.start_untyped_write_nb(*req, dest_proc, dest_rptr, src_lptr, num_bytes);
        return req;
    }

    void untyped_write(
        const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        request_object_type req;
        const auto is_completed =
            self.start_untyped_write_nb(req, dest_proc, dest_rptr, src_lptr, num_bytes);
        
        if (!is_completed) {
            req.wait(self);
        }
    }

private:
    bool start_untyped_write_nb(
        request_object_type&            req_obj
    ,   const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        req_obj.start(self, dest_proc, dest_rptr);
        
        #ifndef MECOM2_RMA_ENABLE_LOOPBACK
        if (self.is_local_proc(dest_proc)) {
            MEFDN_LOG_VERBOSE(
                "msg:Local RMA write (memcpy).\t"
                "dest_rptr:{}\t"
                "src_lptr:{}\t"
                "num_bytes:{}"
            ,   mefdn::show_param(dest_rptr.get_ptr())
            ,   mefdn::show_param(src_lptr.get())
            ,   num_bytes
            );
            
            // Local write.
            std::memcpy(dest_rptr.get_ptr(), src_lptr.get(), num_bytes);
            
            req_obj.finish(self);
            return true;
        }
        #endif
        
        // TODO: meqdc cannot deep-copy iov
        //uct_iov iov = uct_iov();
        auto& iov = req_obj.get_iov();
        iov.buffer = const_cast<void*>(src_lptr.get());
        iov.length = num_bytes;
        iov.memh = src_lptr.get_minfo()->mem.get();
        iov.stride = 0;
        iov.count = 1;
        
        auto& ep = req_obj.get_ep();
        const auto rkey = req_obj.get_rkey_bundle().rkey;
        const auto comp = req_obj.get_completion_ptr();
        ucs_status_t st = UCS_OK;
        {
            const auto lk = req_obj.lock_worker_lock();
            
            st = ep.put_zcopy(
                &iov
            ,   1
            ,   dest_rptr.get_addr()
            ,   rkey
            ,   comp
            );
        }
        
        if (st == UCS_OK) {
            req_obj.finish(self);
            return true;
        }
        else {
            return false;
        }
    }

public:
    unique_request_type untyped_read_nb(
        const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        auto req = self.allocate_unique_request();
        self.start_untyped_read_nb(*req, src_proc, src_rptr, dest_lptr, num_bytes);
        return req;
    }

    void untyped_read(
        const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        request_object_type req;
        const auto is_completed =
            self.start_untyped_read_nb(req, src_proc, src_rptr, dest_lptr, num_bytes);
        
        if (!is_completed) {
            req.wait(self);
        }
    }

private:
    bool start_untyped_read_nb(
        request_object_type&            req_obj
    ,   const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        req_obj.start(self, src_proc, src_rptr);

        #ifndef MECOM2_RMA_ENABLE_LOOPBACK
        if (self.is_local_proc(src_proc)) {
            MEFDN_LOG_VERBOSE(
                "msg:Local RMA read (memcpy).\t"
                "src_rptr:{}\t"
                "dest_lptr:{}\t"
                "num_bytes:{}"
            ,   mefdn::show_param(src_rptr.get_ptr())
            ,   mefdn::show_param(dest_lptr.get())
            ,   num_bytes
            );
            
            // Local read.
            std::memcpy(dest_lptr.get(), src_rptr.get_ptr(), num_bytes);
            
            req_obj.finish(self);
            return true;
        }
        #endif

        auto& ep = req_obj.get_ep();
        const auto rkey = req_obj.get_rkey_bundle().rkey;
        const auto comp = req_obj.get_completion_ptr();
        ucs_status_t st = UCS_OK;
        
        // TODO: Read threshold from UCT config
        if (num_bytes > MECOM2_RMA_UCT_GET_ZCOPY_SIZE) {
            // TODO: meqdc cannot deep-copy iov
            //uct_iov iov = uct_iov();
            auto& iov = req_obj.get_iov();
            iov.buffer = dest_lptr.get();
            iov.length = num_bytes;
            iov.memh = dest_lptr.get_minfo()->mem.get();
            iov.stride = 0;
            iov.count = 1;
            
            const auto lk = req_obj.lock_worker_lock();
                
            st = ep.get_zcopy(
                &iov
            ,   1
            ,   src_rptr.get_addr()
            ,   rkey
            ,   comp
            );
        }
        else {
            const auto lk = req_obj.lock_worker_lock();
            
            st = ep.get_bcopy(
                reinterpret_cast<uct_unpack_callback_t>(&std::memcpy)
            ,   dest_lptr
            ,   num_bytes
            ,   src_rptr.get_addr()
            ,   rkey
            ,   comp
            );
        }
        
        if (st == UCS_OK) {
            req_obj.finish(self);
            return true;
        }
        else {
            return false;
        }
    }

public:
    template <typename T>
    unique_request_type compare_and_swap_nb(
        const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const T* const              expected_lptr
    ,   const T* const              desired_lptr
    ,   T* const                    result_lptr
    ) {
        auto& self = this->derived();
        
        auto req = self.allocate_unique_request();
        self.start_compare_and_swap_nb(*req, target_proc, target_rptr,
            expected_lptr, desired_lptr, result_lptr);
        return req;
    }

    template <typename T>
    void compare_and_swap_b(
        const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const T* const              expected_lptr
    ,   const T* const              desired_lptr
    ,   T* const                    result_lptr
    ) {
        auto& self = this->derived();
        
        request_object_type req;
        const auto is_completed =
            self.start_compare_and_swap_nb(req, target_proc, target_rptr,
                expected_lptr, desired_lptr, result_lptr);
        
        if (!is_completed) {
            req.wait(self);
        }
    }

private:
    // Note: Atomic operation don't require local buffer registration.
    template <typename T>
    bool start_compare_and_swap_nb(
        request_object_type&        req_obj
    ,   const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const T* const              expected_lptr
    ,   const T* const              desired_lptr
    ,   T* const                    result_lptr
    ) {
        auto& self = this->derived();
        req_obj.start(self, target_proc, target_rptr);
        
        auto& ep = req_obj.get_ep();
        const auto rkey = req_obj.get_rkey_bundle().rkey;
        const auto comp = req_obj.get_completion_ptr();
        ucs_status_t st = UCS_OK;
        {
            const auto lk = req_obj.lock_worker_lock();
        
            st = ep.atomic_cswap64(
                *expected_lptr          // compare
            ,   *desired_lptr           // swap
            ,   target_rptr.get_addr()  // remote_addr
            ,   rkey                    // rkey
            ,   result_lptr             // result
            ,   comp                    // comp
            );
        }
        
        if (st == UCS_OK) {
            req_obj.finish(self);
            return true;
        }
        else {
            return false;
        }
    }
};

} // namespace mecom2
} // namespace menps

