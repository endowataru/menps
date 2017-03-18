
#pragma once

#include <mgth/ult/thread.hpp>

namespace mgth {
namespace ult {

struct parallel_execution_policy { };

namespace /*unnamed*/ {

MGBASE_UNUSED parallel_execution_policy par;

} // unnamed namespace

template <typename I, typename F>
void for_loop(parallel_execution_policy /*ignored*/, I first, I last, F func)
{
    // TODO : replace with a recursive version
    
    struct closure {
        F f;
        I i;
        void operator() () {
            f(i);
        }
    };
    
    const auto n = last - first;
    
    // TODO: depends on VLA, which is a compiler extension
    thread t[n];
    
    for (I i = 0; i < n; ++i) {
        t[i] = thread(closure{ func, i+first });
    }
    for (I i = 0; i < n; ++i) {
        t[i].join();
    }
}

} // namespace ult
} // namespace mgth

