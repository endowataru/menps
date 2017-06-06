
#include <mgth.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

void f()
{
    // do nothing
}

int mgth_main(const int argc, char** const argv)
{
    if (argc != 2) {
        std::cerr << argv[0] << " [n]" << std::endl;
        return EXIT_FAILURE;
    }
    
    const auto n = static_cast<mgbase::size_t>(std::atoi(argv[1]));
    
    mgbase::stopwatch sw;
    sw.start();
    
    for (mgbase::size_t i = 0; i < n; ++i) {
        mgth::ult::thread th(f);
        th.join();
    }
    
    const auto elapsed = sw.elapsed();
    
    std::cout << "took " << elapsed << " cycles" << std::endl;
    
    return 0;
}

