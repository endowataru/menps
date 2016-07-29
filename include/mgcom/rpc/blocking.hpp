
#pragma once

#include "call.hpp"
#include <mgbase/ult.hpp>

namespace mgcom {
namespace rpc {

namespace /*unnamed*/ {

template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
,   const mgbase::callback<void ()>&        on_complete
) {
    while (!try_remote_call_async<Handler>(
        proc
    ,   arg
    ,   return_result
    ,   on_complete
    )) {
        mgbase::ult::this_thread::yield();
    }
}
template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call_async(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   const mgbase::callback<void ()>&        on_complete
) {
    while (!try_remote_call_async<Handler>(
        proc
    ,   arg
    ,   on_complete
    )) {
        mgbase::ult::this_thread::yield();
    }
}

template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
,   typename Handler::return_type* const    return_result
) {
    mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_call_async<Handler>(
        proc
    ,   arg
    ,   return_result
    ,   mgbase::make_callback_store_release(&flag, MGBASE_NONTYPE(true))
    );
    
    while (!flag.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
    
    //mgbase::atomic_thread_fence(mgbase::memory_order_seq_cst); // for debugging
}
template <typename Handler>
MGBASE_ALWAYS_INLINE void remote_call(
    const process_id_t                      proc
,   const typename Handler::argument_type&  arg
) {
    mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
    
    remote_call_async<Handler>(
        proc
    ,   arg
    ,   mgbase::make_callback_store_release(&flag, MGBASE_NONTYPE(true))
    );
    
    while (!flag.load(mgbase::memory_order_acquire)) {
        mgbase::ult::this_thread::yield();
    }
}

} // unnamed namespace

} // namespace rpc
} // namespace mgcom

