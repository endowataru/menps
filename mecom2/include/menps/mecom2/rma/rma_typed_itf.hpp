
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/type_traits.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_typed_itf
{
    MEFDN_DEFINE_DERIVED(P)
    
    using proc_id_type = typename P::proc_id_type;
    using size_type = typename P::size_type;
    
public:
    template <typename RemotePtr, typename LocalPtr>
    void write(
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
        
        self.untyped_write(
            dest_proc
        ,   mefdn::forward<RemotePtr>(dest_rptr)
        ,   mefdn::forward<LocalPtr>(src_lptr)
        ,   num_bytes
        );
    }
    
    template <typename RemotePtr, typename LocalPtr>
    void read(
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
        
        self.untyped_read(
            src_proc
        ,   mefdn::forward<RemotePtr>(src_rptr)
        ,   mefdn::forward<LocalPtr>(dest_lptr)
        ,   num_bytes
        );
    }
    
    template <typename Target, typename Value, typename Result>
    void exchange(
        const proc_id_type  target_proc
    ,   Target&&            target_rptr
    ,   Value&&             value_lptr
    ,   Result&&            result_lptr
    ) {
        using target_ptr_type = mefdn::decay_t<Target>;
        using elem_type = typename P::template element_type_of<target_ptr_type>;
        MEFDN_STATIC_ASSERT(!mefdn::is_const<elem_type>::value);
        
        auto& self = this->derived();
        
        self.template exchange_b<elem_type>(
            target_proc
        ,   mefdn::forward<Target>(target_rptr)
        ,   mefdn::forward<Value>(value_lptr)
        ,   mefdn::forward<Result>(result_lptr)
        );
    }
    
    template <typename Target, typename Expected,
        typename Desired, typename Result>
    void compare_and_swap(
        const proc_id_type  target_proc
    ,   Target&&            target_rptr
    ,   Expected&&          expected_lptr
    ,   Desired&&           desired_lptr
    ,   Result&&            result_lptr
    ) {
        using target_ptr_type = mefdn::decay_t<Target>;
        using elem_type = typename P::template element_type_of<target_ptr_type>;
        MEFDN_STATIC_ASSERT(!mefdn::is_const<elem_type>::value);
        
        auto& self = this->derived();
        
        self.template compare_and_swap_b<elem_type>(
            target_proc
        ,   mefdn::forward<Target>(target_rptr)
        ,   mefdn::forward<Expected>(expected_lptr)
        ,   mefdn::forward<Desired>(desired_lptr)
        ,   mefdn::forward<Result>(result_lptr)
        );
    }
    
    template <typename Target, typename T>
    T exchange(
        const proc_id_type  target_proc
    ,   Target&&            target_rptr
    ,   const T             value
    ) {
        T result = T();
        
        this->exchange(
            target_proc
        ,   target_rptr
        ,   &value
        ,   &result
        );
        
        return result;
    }
    
    template <typename Target, typename T>
    T compare_and_swap(
        const proc_id_type  target_proc
    ,   Target&&            target_rptr
    ,   const T             expected
    ,   const T             desired
    ) {
        T result = T();
        
        this->compare_and_swap(
            target_proc
        ,   target_rptr
        ,   &expected
        ,   &desired
        ,   &result
        );
        
        return result;
    }
    
    
    #if 0
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
    #endif
};

} // namespace mecom2
} // namespace menps

