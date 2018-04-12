
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/external/fmt.hpp>

MEOMP_GLOBAL_VAR omp_lock_t lk;
MEOMP_GLOBAL_VAR int var = 0;

extern "C"
int meomp_main(int argc, char** argv)
{
    #pragma omp parallel
    {
        for (int i = 0; i < 100; ++i) {
            omp_set_lock(&lk);
            ++var;
            omp_unset_lock(&lk);
        }
        #pragma omp barrier
    }
    
    fmt::print("ret={}\n", var);
    
    return 0;
}

