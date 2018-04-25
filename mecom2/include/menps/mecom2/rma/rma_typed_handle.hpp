
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/type_traits.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_typed_handle
{
    MEFDN_DEFINE_DERIVED(P)
    
public:
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
    template <typename RemotePtr, typename LocalPtr>
    void read_nb(
        const proc_id_type  src_proc
    ,   RemotePtr&&         src_rptr
    ,   LocalPtr&&          dest_lptr
    ,   const size_type     num_elems
    ) {
        using remote_ptr_type = mefdn::decay_t<RemotePtr>;
        using local_ptr_type  = mefdn::decay_t<LocalPtr>;
        using remote_elem_type = typename P::template element_type_of<remote_ptr_type>;
        using local_elem_type  = typename P::template element_type_of<local_ptr_type>;
        
        // Note: Important for type safety.
        MEFDN_STATIC_ASSERT_MSG(
            (mefdn::is_same<
                mefdn::remove_const_t<remote_elem_type>
            ,   local_elem_type
            >::value)
        ,   "Breaking type safety"
        );
        
        auto& self = this->derived();
        
        const auto num_bytes = num_elems * sizeof(remote_elem_type);
        
        self.untyped_read_nb(
            src_proc
        ,   mefdn::forward<RemotePtr>(src_rptr)
        ,   mefdn::forward<LocalPtr>(dest_lptr)
        ,   num_bytes
        );
    }
    
    template <typename RemotePtr, typename LocalPtr>
    void write_nb(
        const proc_id_type  dest_proc
    ,   RemotePtr&&         dest_rptr
    ,   LocalPtr&&          src_lptr
    ,   const size_type     num_elems
    ) {
        using remote_ptr_type = mefdn::decay_t<RemotePtr>;
        using local_ptr_type  = mefdn::decay_t<LocalPtr>;
        using remote_elem_type = typename P::template element_type_of<remote_ptr_type>;
        using local_elem_type  = typename P::template element_type_of<local_ptr_type>;
        
        // Note: Important for type safety.
        MEFDN_STATIC_ASSERT_MSG(
            (mefdn::is_same<
                remote_elem_type
            ,   mefdn::remove_const_t<local_elem_type>
            >::value)
        ,   "Breaking type safety"
        );
        
        auto& self = this->derived();
        
        const auto num_bytes = num_elems * sizeof(remote_elem_type);
        
        self.untyped_write_nb(
            dest_proc
        ,   mefdn::forward<RemotePtr>(dest_rptr)
        ,   mefdn::forward<LocalPtr>(src_lptr)
        ,   num_bytes
        );
    }
    
protected:
    template <typename TargetPtr, typename ExpectedPtr, typename DesiredPtr, typename ResultPtr>
    void check_cas_type_safety()
    {
        using target_ptr_type   = mefdn::decay_t<TargetPtr>;
        using expected_ptr_type = mefdn::decay_t<ExpectedPtr>;
        using desired_ptr_type  = mefdn::decay_t<DesiredPtr>;
        using result_ptr_type   = mefdn::decay_t<ResultPtr>;
        
        using target_elem_type   = typename P::template element_type_of<target_ptr_type>;
        using expected_elem_type = typename P::template element_type_of<expected_ptr_type>;
        using desired_elem_type  = typename P::template element_type_of<desired_ptr_type>;
        using result_elem_type   = typename P::template element_type_of<result_ptr_type>;
        
        MEFDN_STATIC_ASSERT_MSG(
            (mefdn::is_same<
                target_elem_type
            ,   mefdn::remove_const_t<expected_elem_type>
            >::value)
        ,   "Breaking type safety"
        );
        
        MEFDN_STATIC_ASSERT_MSG(
            (mefdn::is_same<
                target_elem_type
            ,   mefdn::remove_const_t<desired_elem_type>
            >::value)
        ,   "Breaking type safety"
        );
        
        MEFDN_STATIC_ASSERT_MSG(
            (mefdn::is_same<
                target_elem_type
            ,   result_elem_type
            >::value)
        ,   "Breaking type safety"
        );
    }
};

} // namespace mecom2
} // namespace menps

