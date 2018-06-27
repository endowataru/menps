
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
void for_loop_strided(mefdn::execution::sequenced_policy seq,
    I first, I last, S stride, F func)
{
    mefdn::for_loop_strided(first, last, stride, func);
}

template <typename I, typename S, typename F>
void for_loop_strided(mefdn::execution::parallel_policy par,
    I first, I last, S stride, F func)
{
    const auto diff = last - first;
    
    if (diff <= 0) {
        return;
    }
    else if (diff <= stride) {
        func(first);
    }
    else {
        struct starter {
            mefdn::execution::parallel_policy par;
            I first; I last; S stride; F func;
            void operator() () {
                for_loop_strided(par, first, last, stride, func);
            }
        };
        const auto half = diff / 2 / stride * stride;
        thread t(starter{ par, first, first + half, stride, func });
        //for_loop_strided(par, first, first + half, stride, func);
        for_loop_strided(par, first + half, last, stride, func);
        t.join();
    }
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

