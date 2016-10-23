
#include <mgth.hpp>



int mgth_main(const int argc, char** const argv)
{
    auto p = mgth::dsm::make_unique<int []>(100);
    
    p[99] = 1;
    
    return 0;
}

