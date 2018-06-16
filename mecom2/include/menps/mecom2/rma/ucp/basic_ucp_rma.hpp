
#pragma once

#include <menps/mecom2/rma/rma_typed_itf.hpp>
#include <menps/mecom2/rma/rma_pass_buf_copier.hpp>
#include <menps/medev2/ucx/ucp/ucp.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class basic_ucp_rma
    : public rma_typed_itf<P>
    , public rma_pass_buf_copier<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using ult_itf_type = typename P::ult_itf_type;
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using local_ptr_t = typename P::template local_ptr<T>;
    
    using rkey_info_type = typename P::rkey_info_type;
    
    //using req_data_type = typename P::req_data_type;
    
public:
    void untyped_write(
        const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        const auto& info = self.get_rkey_info(dest_proc, dest_rptr);
        
        const auto st_ptr =
            info.ep.put_nb(
                src_lptr
            ,   num_bytes
            ,   dest_rptr.get_addr()
            ,   info.rkey.get()
            ,   &wait_handler
            );
        
        this->wait(info, st_ptr);
    }
    
    void untyped_read(
        const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        const auto& info = self.get_rkey_info(src_proc, src_rptr);
        
        const auto st_ptr =
            info.ep.get_nb(
                dest_lptr
            ,   num_bytes
            ,   src_rptr.get_addr()
            ,   info.rkey.get()
            ,   &wait_handler
            );
        
        this->wait(info, st_ptr);
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
        const auto& info = self.get_rkey_info(target_proc, target_rptr);
        
        // Copy "desired" to "result" because UCP assumes this.
        *result_lptr = *desired_lptr;
        
        const auto st_ptr =
            info.ep.atomic_fetch_nb(
                UCP_ATOMIC_FETCH_OP_CSWAP   // opcode
            ,   *expected_lptr              // value
            ,   result_lptr                 // result (inout)
            ,   sizeof(T)                   // op_size
            ,   target_rptr.get_addr()      // remote_addr
            ,   info.rkey.get()             // rkey
            ,   &wait_handler               // cb
            );
        
        this->wait(info, st_ptr);
    }
    
private:
    void wait(const rkey_info_type& info, const ucs_status_ptr_t st_ptr)
    {
        if (st_ptr == reinterpret_cast<ucs_status_ptr_t>(UCS_OK)) {
            return; 
        }
        
        auto& self = this->derived();
        auto& uf = self.get_ucp_facade();
        
        #if 1
        while (true) {
            info.wk.progress(uf);
            
            const auto chk_st = uf.request_check_status({ st_ptr });
            if (chk_st != UCS_INPROGRESS) {
                break;
            }
            
            ult_itf_type::this_thread::yield();
        }
        #else
        auto& req = * reinterpret_cast<req_data_type*>(st_ptr);
        req.uv.wait();
        #endif
        
        uf.request_free({ st_ptr });
    }
    
    static void wait_handler(void* const /*request*/, const ucs_status_t status)
    {
        if (status != UCS_OK) {
            P::throw_error("error in wait_handler", status);
        }
        
        #if 0
        req.uv.notify();
        #endif
    }
};

} // namespace mecom2
} // namespace menps

