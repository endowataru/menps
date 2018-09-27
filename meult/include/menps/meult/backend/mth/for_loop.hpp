
#pragma once

#include <menps/meult/backend/mth/thread.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/execution.hpp>
#include <menps/mefdn/for_loop.hpp>

namespace menps {
namespace meult {
namespace backend {
namespace mth {

template <typename I, typename S, typename F>
void for_loop_strided(mefdn::execution::sequenced_policy /*seq*/,
    I first, I last, S stride, F func)
{
    mefdn::for_loop_strided(first, last, stride, func);
}

namespace detail {

template <typename I, typename S, typename F>
struct for_loop_par_params {
    I first; I a; I b; S stride; const F& func;
};

template <typename I, typename S, typename F>
void* for_loop_par_aux(void* const arg)
{
    using params_type = for_loop_par_params<I, S, F>;
    const auto& p = static_cast<const params_type*>(arg);
    
    const auto first = p->first;
    const auto a = p->a;
    const auto b = p->b;
    const auto stride = p->stride;
    const auto& func = p->func;
    
    const auto d = b - a;
    MEFDN_ASSERT(d > 0);
    
    if (d == 1) {
        func(first + a * stride);
    }
    else {
        const auto c = a + d / 2;
        params_type p0{ first, a, c, stride, func };
        auto t = thread(fork_fast(&for_loop_par_aux<I, S, F>, &p0));
        
        params_type p1{ first, c, b, stride, func };
        for_loop_par_aux<I, S, F>(&p1);
        
        t.join();
    }
    
    return nullptr;
}

} // namespace detail

template <typename I, typename S, typename F>
void for_loop_strided(mefdn::execution::parallel_policy /*par*/,
    I first, I last, S stride, const F& func)
{
    if (last - first == 0) {
        return;
    }
    
    detail::for_loop_par_params<I, S, F> p{
        first,
        0, (last - first + stride - 1) / stride,
        stride, func
    };
    detail::for_loop_par_aux<I, S, F>(&p);
}

template <typename ExecutionPolicy, typename I, typename F>
void for_loop(ExecutionPolicy&& exec, I first, I last, F func)
{
    for_loop_strided(mefdn::forward<ExecutionPolicy>(exec), first, last, 1, func);
}

} // namespace mth
} // namespace backend
} // namespace meult
} // namespace menps

