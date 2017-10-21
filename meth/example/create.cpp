
#include <menps/meth.hpp>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <menps/mefdn/profiling/time.hpp>
#include <iostream>

using namespace menps;

void f()
{
    // do nothing
}

int meth_main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = static_cast<mefdn::size_t>(std::atoi(argv[1]));
    
    mefdn::stopwatch sw;
    sw.start();
    const auto start_sec = mefdn::get_current_sec();
    
    for (mefdn::size_t i = 0; i < n; ++i) {
        meth::ult::thread th(f);
        th.join();
    }
    
    const auto elapsed = sw.elapsed();
    const auto end_sec = mefdn::get_current_sec();
    
    std::cout << "took " << elapsed << " cycles, " << end_sec << " sec" << std::endl;
    
    return 0;
}

