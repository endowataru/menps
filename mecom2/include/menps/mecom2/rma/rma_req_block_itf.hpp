
#pragma once

#include <menps/mecom2/common.hpp>
#include <cstring>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_req_block_itf
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using local_ptr_t = typename P::template local_ptr<T>;
    
    using request_type = typename P::request_type;
    
public:
    void untyped_write(
        const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        #ifndef MECOM2_RMA_ENABLE_LOOPBACK
        if (self.is_local_proc(dest_proc)) {
            MEFDN_LOG_VERBOSE(
                "msg:Local RMA write (memcpy).\t"
                "dest_rptr:{}\t"
                "src_lptr:{}\t"
                "num_bytes:{}"
            ,   mefdn::show_param(dest_rptr)
            ,   mefdn::show_param(src_lptr)
            ,   num_bytes
            );
            
            // Local write.
            std::memcpy(dest_rptr, src_lptr, num_bytes);
            
            return;
        }
        #endif
        
        request_type req;
        
        self.untyped_write_nb(
            dest_proc
        ,   dest_rptr
        ,   src_lptr
        ,   num_bytes
        ,   &req
        );
        
        self.wait(&req);
    }
    
    void untyped_read(
        const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        #ifndef MECOM2_RMA_ENABLE_LOOPBACK
        if (self.is_local_proc(src_proc)) {
            MEFDN_LOG_VERBOSE(
                "msg:Local RMA read (memcpy).\t"
                "src_rptr:{}\t"
                "dest_lptr:{}\t"
                "num_bytes:{}"
            ,   mefdn::show_param(src_rptr)
            ,   mefdn::show_param(dest_lptr)
            ,   num_bytes
            );
            
            // Local read.
            std::memcpy(dest_lptr, src_rptr, num_bytes);
            
            return;
        }
        #endif
        
        request_type req;
        
        self.untyped_read_nb(
            src_proc
        ,   src_rptr
        ,   dest_lptr
        ,   num_bytes
        ,   &req
        );
        
        self.wait(&req);
    }
    
    template <typename T>
    void exchange_b(
        const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const local_ptr_t<const T>& value_lptr
    ,   const local_ptr_t<T>&       result_lptr
    ) {
        auto& self = this->derived();
        
        request_type req;
        
        self.exchange_nb(
            target_proc
        ,   target_rptr
        ,   value_lptr
        ,   result_lptr
        ,   &req
        );
        
        self.wait(&req);
    }
    
    template <typename T>
    void compare_and_swap_b(
        const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const local_ptr_t<const T>& expected_lptr
    ,   const local_ptr_t<const T>& desired_lptr
    ,   const local_ptr_t<T>&       result_lptr
    ) {
        auto& self = this->derived();
        
        request_type req;
        
        self.compare_and_swap_nb(
            target_proc
        ,   target_rptr
        ,   expected_lptr
        ,   desired_lptr
        ,   result_lptr
        ,   &req
        );
        
        self.wait(&req);
    }
};

} // namespace mecom2
} // namespace menps


