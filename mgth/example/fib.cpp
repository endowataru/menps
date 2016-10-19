
#include <mgth.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace /*unnamed*/ {

// Fibonacci

typedef mgbase::uint64_t    fib_int_t;

struct fib_data
{
    fib_int_t*  ret;
    fib_int_t   arg;
};

void fib(void* const arg)
{
    namespace sched = mgth::sched;
    
    const auto& d = *static_cast<fib_data*>(arg);
    
    const auto n = d.arg;
    
    if (n == 0 || n == 1) {
        *d.ret = n;
        return;
    }
    
    auto t1 = sched::allocate_thread(MGBASE_ALIGNOF(fib_data), sizeof(fib_data));
    auto t2 = sched::allocate_thread(MGBASE_ALIGNOF(fib_data), sizeof(fib_data));
    
    auto& d1 = *static_cast<fib_data*>(t1.ptr);
    auto& d2 = *static_cast<fib_data*>(t2.ptr);
    
    fib_int_t r1{};
    fib_int_t r2{};
    
    d1 = { &r1, n-1 };
    d2 = { &r2, n-2 };
    
    sched::fork(t1, &fib);
    sched::fork(t2, &fib);
    
    sched::join(t1.id);
    sched::join(t2.id);
    
    *d.ret = r1 + r2;
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
    
    fib_int_t result{};
    
    fib_data d{ &result, arg_n };
    
    fib(&d);
    
    std::cout << "fib(" << arg_n << ") = " << result
        << ", took " << sw.elapsed() << " cycles" << std::endl;
    
    return 0;
}

