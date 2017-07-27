
#include <mgth.hpp>
#include <mgcom/common.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <mgbase/profiling/time.hpp>
#include <iostream>
#include <fstream>
#include <mgbase/external/fmt.hpp>

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
    
    //std::cout << n << " : " << r1 << " " << r2 << std::endl;
    
    return r1 + r2;
}

} // unnamed namespace

int mgth_main(const int argc, char** const argv)
{
    if (argc != 3) {
        std::cerr << argv[0] << " [n] [out]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = static_cast<mgbase::uintptr_t>(std::atoi(argv[1]));
    //std::cout << argv[0] << " " << argv[1] << " " << argv[2];
    
    mgbase::stopwatch sw;
    sw.start();
    const auto start_sec = mgbase::get_current_sec();
    
    const auto result = fib(n);
    
    const auto cycles = sw.elapsed();
    const auto end_sec = mgbase::get_current_sec();
    
    using fmt::print;
    
    {
        std::fstream ofs(argv[2], std::fstream::out | std::fstream::app);
        fmt::MemoryWriter w;
        
        w.write(/*ofs,*/ "- exp_type: fib\n");
        w.write(/*ofs,*/ "  n: {}\n", n);
        w.write(/*ofs,*/ "  number_of_processes: {}\n", mgcom::number_of_processes());
        w.write(/*ofs,*/ "  cycles: {} # [cycles]\n", cycles);
        w.write(/*ofs,*/ "  duration: {} # [sec]\n", (end_sec - start_sec));
        
        const auto s = w.str();
        
        std::cout << s;
        
        //std::fstream ofs(argv[2], std::fstream::out | std::fstream::app);
        //ofs << s;
        //ofs << std::endl;
        
        //std::cout << argv[2];
    }
    
    return 0;
}
