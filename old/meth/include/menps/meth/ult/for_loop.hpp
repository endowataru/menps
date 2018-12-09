
#pragma once

#include <menps/meth/ult/thread.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace meth {
namespace ult {

struct parallel_execution_policy { };

namespace /*unnamed*/ {

MEFDN_MAYBE_UNUSED parallel_execution_policy par;

} // unnamed namespace

template <typename I, typename S, typename F>
void for_loop_strided(parallel_execution_policy /*ignored*/, I first, I last, S stride, F func)
{
    // TODO : replace with a recursive version
    
    struct closure {
        F f;
        I i;
        void operator() () {
            f(i);
        }
    };
    
    const auto n = mefdn::roundup_divide((last - first), stride);
    
    // TODO: depends on VLA, which is a compiler extension
    thread t[n];
    
    for (I i = 0; i < n; ++i) {
        t[i] = thread(closure{ func, (i*stride + first) });
    }
    for (I i = 0; i < n; ++i) {
        t[i].join();
    }
}

template <typename ExecutionPolicy, typename I, typename F>
void for_loop(ExecutionPolicy&& exec, I first, I last, F func)
{
    for_loop_strided(mefdn::forward<ExecutionPolicy>(exec), first, last, 1, func);
}

} // namespace ult
} // namespace meth
} // namespace menps

