
#include <menps/meth.hpp>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <menps/mefdn/profiling/time.hpp>
#include <iostream>

using namespace menps;

namespace /*unnamed*/ {

// Fibonacci

typedef mefdn::uint64_t    fib_int_t;

fib_int_t fib(const fib_int_t n)
{
    if (n == 0 || n == 1) {
        return n;
    }
    
    meth::ult::scoped_task<fib_int_t> t(&fib, n - 1);
    
    const auto r2 = fib(n - 2);
    
    const auto r1 = t.get();
    
    std::cout << n << " : " << r1 << " " << r2 << std::endl;
    
    return r1 + r2;
}

} // unnamed namespace

int meth_main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = static_cast<mefdn::uintptr_t>(std::atoi(argv[1]));
    
    mefdn::stopwatch sw;
    sw.start();
    const auto start_sec = mefdn::get_current_sec();
    
    const auto result = fib(n);
    
    const auto elapsed = sw.elapsed();
    const auto end_sec = mefdn::get_current_sec();
    
    std::cout << "fib(" << n << ") = " << result
        << ", took " << elapsed << " cycles, "
        << (end_sec - start_sec) << " sec" << std::endl;
    
    return 0;
}

