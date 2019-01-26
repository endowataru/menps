
#pragma once

#include "mth_interface.hpp"
#include "barrier.hpp"
#include <menps/meult/ult_itf_id.hpp>

namespace menps {
namespace meult {

namespace backend {
namespace mth {

struct ult_policy
{
    typedef meult::backend::mth::mutex              mutex;
    typedef meult::backend::mth::condition_variable condition_variable;
    
    typedef meult::backend::mth::unique_lock<meult::backend::mth::mutex>    unique_mutex_lock;
    
    struct this_thread
    {
        static void yield() {
            meult::backend::mth::this_thread::yield();
        }
        
        static void detach() {
            meult::backend::mth::this_thread::detach();
        }
        
        static myth_thread_t native_handle() {
            return myth_self();
        }
    };
    
    static myth_thread_t fork_fast(void* (* const func)(void*), void* const arg) {
        return meult::backend::mth::fork_fast(func, arg);
    }
    
    typedef mefdn::spinlock            spinlock;
    
    typedef meult::backend::mth::thread thread;
    
    template <typename Policy>
    struct thread_specific_ {
        typedef typename meult::backend::mth::thread_specific<Policy>   type;
    };
    
    // Note: The type "P" is used to distinguish thread-local storages with the same type.
    template <typename P>
    using thread_specific =
        typename meult::backend::mth::thread_specific<P>;
    
    typedef meult::backend::mth::uncond_variable    uncond_variable;
    
    using barrier = mth::barrier;
    
    struct execution
    {
        static constexpr mefdn::execution::sequenced_policy seq{};
        static constexpr mefdn::execution::parallel_policy par{};
    };
    
    
    template <typename ExecutionPolicy,
        typename I, typename S, typename... Rest>
    static void for_loop_strided(
        ExecutionPolicy &&              exec
    ,   const mefdn::type_identity_t<I> start
    ,   const I                         finish
    ,   const S                         stride
    ,   Rest && ...                     rest
    ) {
        mth::for_loop_strided(
            mefdn::forward<ExecutionPolicy>(exec)
        ,   start
        ,   finish
        ,   stride
        ,   mefdn::forward<Rest>(rest)...
        );
    }
    
    template <typename ExecutionPolicy,
        typename I, typename... Rest>
    static void for_loop(
        ExecutionPolicy &&              exec
    ,   const mefdn::type_identity_t<I> start
    ,   const I                         finish
    ,   Rest && ...                     rest
    ) {
        mth::for_loop(
            mefdn::forward<ExecutionPolicy>(exec)
        ,   start
        ,   finish
        ,   mefdn::forward<Rest>(rest)...
        );
    }
    
    #ifdef MEULT_ENABLE_MTH_WORKER_CACHE
    static mefdn::size_t get_worker_num() noexcept
    {
        static MEFDN_THREAD_LOCAL mefdn::size_t wk_num_ = 0;
        auto wk_num = wk_num_;
        if (MEFDN_UNLIKELY(wk_num == 0)) {
            wk_num = static_cast<mefdn::size_t>(myth_get_worker_num())+1;
            wk_num_ = wk_num;
        }
        return wk_num-1;
    }
    #else
    static mefdn::size_t get_worker_num() noexcept
    {
        return myth_get_worker_num();
    }
    #endif
    static mefdn::size_t get_num_workers() noexcept
    {
        return mth::get_num_workers();
    }
};

} // namespace mth
} // namespace backend

template <>
struct get_ult_itf_type<ult_itf_id_t::MTH>
    : mefdn::type_identity<
        backend::mth::ult_policy
    >
{ };

} // namespace meult
} // namespace menps

