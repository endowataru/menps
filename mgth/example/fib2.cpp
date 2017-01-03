
#include <mgth.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace /*unnamed*/ {

// Fibonacci

typedef mgbase::uint64_t    fib_int_t;

fib_int_t fib(const fib_int_t n)
{
    if (n == 0 || n == 1) {
        return n;
    }
    
    mgth::ult::scoped_task<fib_int_t> t(&fib, n - 1);
    
    const auto r2 = fib(n - 2);
    
    const auto r1 = t.get();
    
    std::cout << n << " : " << r1 << " " << r2 << std::endl;
    
    return r1 + r2;
}

} // unnamed namespace

int mgth_main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = static_cast<mgbase::uintptr_t>(std::atoi(argv[1]));
    
    mgbase::stopwatch sw;
    sw.start();
    
    const auto result = fib(n);
    
    const auto elapsed = sw.elapsed();
    
    std::cout << "fib(" << n << ") = " << result
        << ", took " << elapsed << " cycles" << std::endl;
    
    return 0;
}

