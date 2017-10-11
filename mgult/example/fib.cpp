
#include <mgult/sm.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace /*unnamed*/ {

// Fibonacci

typedef mgbase::uint64_t    fib_int_t;

struct fib_func
{
    fib_int_t*  ret;
    fib_int_t   n;
    
    void operator () ()
    {
        if (n == 0 || n == 1) {
            *ret = n;
            return;
        }
        
        fib_int_t r1{};
        fib_int_t r2{};
        
        mgult::sm::thread th{ fib_func{ &r1, n-1 } };
        
        fib_func{ &r2, n-2 }();
        
        th.join();
        
        *ret = r1 + r2;
    }
};

fib_int_t start_fib(const fib_int_t n)
{
    fib_int_t result = 0;
    
    fib_func{ &result, n }();
    
    return result;
}

} // unnamed namespace

int main(const int argc, char** const argv)
{
    mgult::sm::initializer init;
    
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = 
        static_cast<mgbase::uintptr_t>(std::atoi(argv[1]));
    
    mgbase::stopwatch sw;
    sw.start();
    
    const auto result = start_fib(n);
    
    std::cout << "fib(" << n << ") = " << result
        << ", took " << sw.elapsed() << " cycles" << std::endl;
    
    return 0;
}

