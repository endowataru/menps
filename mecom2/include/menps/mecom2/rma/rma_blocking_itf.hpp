
#pragma once

#include <menps/mecom2/rma/basic_rma_itf.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_blocking_itf
    : public basic_rma_itf<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = basic_rma_itf<P>;
    
    using size_type       = typename P::size_type;
    
public:
    using typename base::proc_id_type;
    
    template <typename RemotePtr, typename LocalPtr>
    void read(
        const proc_id_type  src_proc
    ,   RemotePtr&&         src_rptr
    ,   LocalPtr&&          dest_lptr
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        
        auto h = self.make_handle();
        
        h.read_nb(
            src_proc
        ,   mefdn::forward<RemotePtr>(src_rptr)
        ,   mefdn::forward<LocalPtr>(dest_lptr)
        ,   num_elems
        );
        
        h.flush();
    }
    
    template <typename RemotePtr, typename LocalPtr>
    void write(
        const proc_id_type  dest_proc
    ,   RemotePtr&&         dest_rptr
    ,   LocalPtr&&          src_lptr
    ,   const size_type     num_elems
    ) {
        auto& self = this->derived();
        
        auto h = self.make_handle();
        
        h.write_nb(
            dest_proc
        ,   mefdn::forward<RemotePtr>(dest_rptr)
        ,   mefdn::forward<LocalPtr>(src_lptr)
        ,   num_elems
        );
        
        h.flush();
    }
    
    template <typename TargetPtr, typename T>
    T compare_and_swap(
        const proc_id_type  target_proc
    ,   TargetPtr&&         target_rptr
    ,   const T             expected
    ,   const T             desired
    ) {
        auto& self = this->derived();
        
        T result{};
        {
            auto h = self.make_handle();
            
            h.compare_and_swap_nb(
                target_proc
            ,   mefdn::forward<TargetPtr>(target_rptr)
            ,   &expected
            ,   &desired
            ,   &result
            );
            
            h.flush();
        }
        
        return result;
    }
};

} // namespace mecom2
} // namespace menps

