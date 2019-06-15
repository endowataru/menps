
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_pass_buf_copier
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename T>
    using remote_ptr_t = typename P::template remote_ptr<T>;
    template <typename T>
    using local_ptr_t = typename P::template local_ptr<T>;
    template <typename T>
    using unique_local_ptr_t = typename P::template unique_local_ptr<T>;
    template <typename Ptr>
    using element_type_of = typename P::template element_type_of<Ptr>;
    
    template <typename Ptr>
    using elem_t = mefdn::remove_const_t<element_type_of<mefdn::decay_t<Ptr>>>;
    
public:
    template <typename RemotePtr>
    unique_local_ptr_t<elem_t<RemotePtr> []>
    buf_read(
        const proc_id_type  src_proc
    ,   RemotePtr&&         src_rptr
    ,   const size_type     num_elems
    ) {
        using element_type = elem_t<RemotePtr>;
        
        auto& self = this->derived();
        
        auto buf =
            self.template make_private_uninitialized<element_type []>(num_elems);
        
        self.read(src_proc, src_rptr, buf.get(), num_elems);
        
        return buf;
    }
    
    template <typename RemotePtr, typename T>
    void buf_write(
        const proc_id_type  dest_proc
    ,   RemotePtr&&         dest_rptr
    ,   const T* const      src_ptr
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        
        self.write(dest_proc, dest_rptr, src_ptr, num_elems);
    }
};

} // namespace mecom2
} // namespace menps
