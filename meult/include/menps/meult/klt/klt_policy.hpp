
#pragma once

#include <menps/meult/klt/klt.hpp>
#include <menps/mefdn/execution.hpp>
#include <menps/mefdn/for_loop.hpp>
#include <menps/meult/ult_itf_id.hpp>

namespace menps {
namespace meult {

struct klt_policy
{
    typedef meult::klt::mutex               mutex;
    typedef meult::klt::condition_variable  condition_variable;
    
    typedef meult::klt::unique_lock<meult::klt::mutex>  unique_mutex_lock;
    
    struct this_thread
    {
        static void yield() {
            meult::klt::this_thread::yield();
        }
        
        static void detach() {
            meult::klt::this_thread::detach();
        }
        
        static pthread_t native_handle() {
            return pthread_self();
        }
    };
    
    typedef meult::klt::spinlock            spinlock;
    
    typedef meult::klt::thread              thread;
    
    template <typename Policy>
    struct thread_specific_ {
        typedef typename meult::klt::thread_specific<Policy>    type;
    };
    
    // Note: The type "P" is used to distinguish thread-local storages with the same type.
    template <typename P>
    using thread_specific =
        typename meult::klt::thread_specific<P>;
    
    using barrier = meult::klt::barrier;
    
    struct execution
    {
        static constexpr mefdn::execution::sequenced_policy seq{};
        static constexpr mefdn::execution::parallel_policy par{};
    };
    
    template <typename ExecutionPolicy,
        typename I, typename... Rest>
    static void for_loop(
        ExecutionPolicy &&              exec
    ,   const mefdn::type_identity_t<I> start
    ,   const I                         finish
    ,   Rest && ...                     rest
    ) {
        // Execute sequentially.
        mefdn::for_loop(
            start
        ,   finish
        ,   mefdn::forward<Rest>(rest)...
        );
    }
    
    static mefdn::size_t get_worker_num() noexcept {
        return 0; // TODO
    }
    static mefdn::size_t get_num_workers() noexcept {
        return klt::get_num_workers();
    }
    
    using uncond_variable = klt::emulated_uncond_variable;
};

namespace klt {

typedef klt_policy  ult_policy;

} // namespace klt

template <>
struct get_ult_itf_type<ult_itf_id_t::KLT>
    : mefdn::type_identity<
        klt_policy
    >
{ };

} // namespace meult
} // namespace menps

