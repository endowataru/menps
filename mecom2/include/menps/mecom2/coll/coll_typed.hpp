
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class coll_typed
{
    MEFDN_DEFINE_DERIVED(P)
    
    using size_type = typename P::size_type;
    
public:
    template <typename T>
    void allgather(
        const T* const  src_ptr
    ,   T* const        dest_ptr
    ,   const size_type num_elems
    ) {
        auto& self = this->derived();
        
        self.untyped_allgather(
            src_ptr
        ,   dest_ptr
        ,   num_elems * sizeof(T)
        );
    }
    
    template <typename T>
    void alltoall(
        const T* const  src_ptr
    ,   T* const        dest_ptr
    ,   const size_type num_elems
    ) {
        auto& self = this->derived();
        
        self.untyped_alltoall(
            src_ptr
        ,   dest_ptr
        ,   num_elems * sizeof(T)
        );
    }
};

} // namespace mecom2
} // namespace menps

