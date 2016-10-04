
#include <mgth.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace /*unnamed*/ {

/* Fibonacci with stack variables */

void* fib(void* const arg)
{
    const auto n = reinterpret_cast<mgbase::uintptr_t>(arg);
    
    if (n == 0 || n == 1) {
        return reinterpret_cast<void*>(n);
    }
    
    const auto t1 = mgth::fork(&fib, reinterpret_cast<void*>(n - 1));
    const auto t2 = mgth::fork(&fib, reinterpret_cast<void*>(n - 2));
    
    const auto r1 = mgth::join(t1);
    const auto r2 = mgth::join(t2);
    
    const auto ri1 = reinterpret_cast<mgbase::uintptr_t>(r1);
    const auto ri2 = reinterpret_cast<mgbase::uintptr_t>(r2);
    
    return reinterpret_cast<void*>(ri1 + ri2);
}

} // unnamed namespace

int mgth_main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto arg_n = static_cast<mgbase::uintptr_t>(std::atoi(argv[1]));
    
    mgbase::stopwatch sw;
    sw.start();
    
    const auto r = fib(reinterpret_cast<void*>(arg_n));
    
    std::cout << "fib(" << arg_n << ") = " << r
        << ", took " << sw.elapsed() << " cycles" << std::endl;
    
    return 0;
}

