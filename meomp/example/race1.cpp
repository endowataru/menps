 
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/external/fmt.hpp>

extern "C"
int meomp_main(int argc, char** argv)
{
    long long var = 123;
    
    #pragma omp parallel
    {
        const auto th_num = omp_get_thread_num();
        fmt::print("{}\n", th_num);
        
        var = th_num;
        
        #pragma omp barrier
    }
    
    fmt::print("Passed: ret={}\n", var);
    
    return 0;
}
 
 
