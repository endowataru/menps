
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <iostream>

namespace example {

namespace mefdn = menps::mefdn;

// Fibonacci

typedef mefdn::uint64_t    fib_int_t;

fib_int_t fib(const fib_int_t n)
{
    if (n == 0 || n == 1) {
        return n;
    }
    
    ult::scoped_task<fib_int_t> t(&fib, n - 1);
    
    const auto r2 = fib(n - 2);
    
    const auto r1 = t.get();
    
    return r1 + r2;
}

fib_int_t fib_seq(const fib_int_t n)
{
    fib_int_t r1 = 0;
    fib_int_t r2 = 1;
    
    for (fib_int_t i = 1; i <= n; ++i) {
        const auto r = r2;
        r2 += r1;
        r1 = r;
    }
    
    return r1;
}

int fib_main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = 
        static_cast<mefdn::uintptr_t>(std::atoi(argv[1]));
    
    mefdn::stopwatch sw;
    sw.start();
    
    const auto result = fib(n);
    
    const auto elapsed = sw.elapsed();
    
    using fmt::print;
    
    print("fib({}) = {}, took {} cycles\n", n, result, elapsed);
    
    const auto ans = fib_seq(n);
    const bool ok = ans == result;
    print("{}. fib({}) is expected to be {}.\n", ok ? "OK" : "Fail", n, ans);
    
    return !ok;
}

} // namespace example

