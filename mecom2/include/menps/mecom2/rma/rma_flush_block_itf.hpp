
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_flush_block_itf
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using local_ptr_t = typename P::template local_ptr<T>;
    
public:
    void untyped_write(
        const proc_id_type              dest_proc
    ,   const remote_ptr_t<void>&       dest_rptr
    ,   const local_ptr_t<const void>&  src_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        self.untyped_write_nb(
            dest_proc
        ,   dest_rptr
        ,   src_lptr
        ,   num_bytes
        );
        
        self.flush_local(dest_proc);
    }
    
    void untyped_read(
        const proc_id_type              src_proc
    ,   const remote_ptr_t<const void>& src_rptr
    ,   const local_ptr_t<void>&        dest_lptr
    ,   const size_type                 num_bytes
    ) {
        auto& self = this->derived();
        
        self.untyped_read_nb(
            src_proc
        ,   src_rptr
        ,   dest_lptr
        ,   num_bytes
        );
        
        self.flush_local(src_proc);
    }
    
    template <typename T>
    void exchange_b(
        const proc_id_type          target_proc
    ,   const remote_ptr_t<T>&      target_rptr
    ,   const local_ptr_t<const T>& value_lptr
    ,   const local_ptr_t<T>&       result_lptr
    ) {
        auto& self = this->derived();
        
        self.exchange_nb(
            target_proc
        ,   target_rptr
        ,   value_lptr
        ,   result_lptr
        );
        
        self.flush_local(target_proc);
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
        
        self.compare_and_swap_nb(
            target_proc
        ,   target_rptr
        ,   expected_lptr
        ,   desired_lptr
        ,   result_lptr
        );
        
        self.flush_local(target_proc);
    }
};

} // namespace mecom2
} // namespace menps
