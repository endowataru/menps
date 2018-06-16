 
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/external/fmt.hpp>

MEOMP_GLOBAL_VAR int g_var = 123;

extern "C"
int meomp_main(int argc, char** argv)
{
    #pragma omp parallel
    {
        const auto th_num = omp_get_thread_num();
        fmt::print("{} {} \n", th_num, g_var);
        
        #pragma omp barrier
        
        g_var = th_num;
        
        #pragma omp barrier
    }
    
    fmt::print("Passed: ret={}\n", g_var);
    
    return 0;
}
 
 
