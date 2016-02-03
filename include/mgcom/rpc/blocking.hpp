
#pragma once

#include "call.hpp"

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
,   const mgbase::operation&                on_complete
) {
    while (!try_remote_call_async<Handler>(
        proc
    ,   arg
    ,   return_result
    ,   on_complete
    ))
    { }
}
template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   const mgbase::operation&                on_complete
) {
    while (!try_remote_call_async<Handler>(
        proc
    ,   arg
    ,   on_complete
    ))
    { }
}

template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
) {
    mgbase::atomic<bool> flag(false);
    
    remote_call_async<Handler>(
        proc
    ,   arg
    ,   return_result
    ,   mgbase::make_operation_store_release(&flag, true)
    );
    
    while (!flag.load(mgbase::memory_order_acquire)) { }
    
    //mgbase::atomic_thread_fence(mgbase::memory_order_seq_cst); // for debugging
}
template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
) {
    mgbase::atomic<bool> flag(false);
    
    remote_call_async<Handler>(
        proc
    ,   arg
    ,   mgbase::make_operation_store_release(&flag, true)
    );
    
    while (!flag.load(mgbase::memory_order_acquire)) { }
    
    //mgbase::atomic_thread_fence(mgbase::memory_order_seq_cst); // for debugging
}

} // unnamed namespace

} // namespace rpc
} // namespace mgcom

