

#include <iostream>
#include <omp.h>

#include <menps/meomp.hpp>

extern "C"
int meomp_main(int argc, char** argv)
{
    meomp_prof_begin();
    for (int i = 0; i < 10; ++i) {
        #pragma omp parallel
        {
            std::cout << omp_get_thread_num() << "/" << omp_get_num_threads()
                << " " << i << std::endl;
            
            #pragma omp barrier
        }
    }
    meomp_prof_end();
}

