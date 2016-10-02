
#include <mgult/sm.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace /*unnamed*/ {

mgult::scheduler_ptr g_s;

/* Fibonacci with stack variables */

void* fib(void* const arg)
{
    const auto n = reinterpret_cast<mgbase::uintptr_t>(arg);
    
    if (n == 0 || n == 1) {
        return reinterpret_cast<void*>(n);
    }
    
    const auto t1 = g_s->fork(&fib, reinterpret_cast<void*>(n - 1));
    const auto t2 = g_s->fork(&fib, reinterpret_cast<void*>(n - 2));
    
    const auto r1 = g_s->join(t1);
    const auto r2 = g_s->join(t2);
    
    const auto ri1 = reinterpret_cast<mgbase::uintptr_t>(r1);
    const auto ri2 = reinterpret_cast<mgbase::uintptr_t>(r2);
    
    return reinterpret_cast<void*>(ri1 + ri2);
}

mgbase::uintptr_t g_result = 0;
mgbase::uintptr_t g_arg_n = 0;

void start_fib()
{
    const auto r = fib(reinterpret_cast<void*>(g_arg_n));
    
    g_result = reinterpret_cast<mgbase::uintptr_t>(r);
}

} // unnamed namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    g_arg_n = static_cast<mgbase::uintptr_t>(std::atoi(argv[1]));
    
    g_s = mgult::make_sm_scheduler();
    
    mgbase::stopwatch sw;
    sw.start();
    
    g_s->loop(start_fib);
    
    std::cout << "fib(" << g_arg_n << ") = " << g_result
        << ", took " << sw.elapsed() << " cycles" << std::endl;
    
    g_s.reset();
    
    return 0;
}

