
#include <menps/meult/sm.hpp>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <iostream>

namespace mefdn = menps::mefdn;
namespace meult = menps::meult;

namespace /*unnamed*/ {

// Fibonacci

typedef mefdn::uint64_t    fib_int_t;

struct fib_data
{
    fib_int_t*  ret;
    fib_int_t   arg;
};

void fib(void* const arg)
{
    const auto& d = *static_cast<fib_data*>(arg);
    
    const auto n = d.arg;
    
    if (n == 0 || n == 1) {
        *d.ret = n;
        return;
    }
    
    auto& s = meult::sm::get_scheduler();
    
    auto t1 = s.allocate(alignof(fib_data), sizeof(fib_data));
    auto t2 = s.allocate(alignof(fib_data), sizeof(fib_data));
    
    auto& d1 = *static_cast<fib_data*>(t1.ptr);
    auto& d2 = *static_cast<fib_data*>(t2.ptr);
    
    fib_int_t r1{};
    fib_int_t r2{};
    
    d1 = { &r1, n-1 };
    d2 = { &r2, n-2 };
    
    s.fork(t1, &fib);
    s.fork(t2, &fib);
    
    s.join(t1.id);
    s.join(t2.id);
    
    *d.ret = r1 + r2;
}

fib_int_t g_result = 0;
fib_int_t g_arg_n = 0;

void start_fib()
{
    fib_data d{ &g_result, g_arg_n };
    
    fib(&d);
}

} // unnamed namespace

int main(int argc, char** argv)
{
    meult::sm::initializer init;
    
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    g_arg_n = static_cast<mefdn::uintptr_t>(std::atoi(argv[1]));
    
    mefdn::stopwatch sw;
    sw.start();
    
    start_fib();
    
    std::cout << "fib(" << g_arg_n << ") = " << g_result
        << ", took " << sw.elapsed() << " cycles" << std::endl;
    
    return 0;
}

