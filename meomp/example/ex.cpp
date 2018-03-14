
#include <iostream>
#include <omp.h>

extern "C"
int meomp_main(int argc, char** argv)
{
    int x = omp_get_thread_num();
    
    #pragma omp parallel
    {
        std::cout << omp_get_thread_num() << "=" << x << std::endl;
        
        #pragma omp for
        for (int i = 0; i < 1000; ++i) {
            std::cout << i << " " << omp_get_thread_num() << std::endl;
        }
        
        //#pragma omp barrier
        
        //std::cout << omp_get_thread_num() << std::endl;
    }
    
    return 0;
}

