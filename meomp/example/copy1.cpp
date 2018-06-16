
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/arithmetic.hpp>

#define MAX_SIZE (1 << 20)

unsigned char g_buf1[MAX_SIZE] MEOMP_GLOBAL_VAR;
unsigned char g_buf2[MAX_SIZE] MEOMP_GLOBAL_VAR;

extern "C"
int meomp_main(int argc, char** argv)
{
    using fmt::print;
    using menps::mefdn::average_accumulator;
    using menps::mefdn::get_cpu_clock;
    using menps::mefdn::cpu_clock_t;
    
    const auto num_trials = std::atoi(argv[1]);
    const auto size = std::atoi(argv[2]);
    
    if (size > MAX_SIZE) {
        print("Too big. max={}\n", MAX_SIZE);
        return 1;
    }
    
    #pragma omp parallel
    {
        average_accumulator<cpu_clock_t> write_acc;
        average_accumulator<cpu_clock_t> bar_acc;
        
        #pragma omp for
        for (int j = 0; j < size; ++j) {
            g_buf1[j] = static_cast<unsigned char>(j + omp_get_thread_num());
        }
        
        const int num_threads = omp_get_num_threads();
        const int thread_id = omp_get_thread_num();
        
        const auto size_per_th = menps::mefdn::roundup_divide(size, num_threads);
        
        for (int i = 0; i < num_trials; ++i) {
            const auto t0 = get_cpu_clock();
            
            const bool turn = i % 2 == 0;
            
            unsigned char* buf_from = turn ? g_buf1 : g_buf2;
            unsigned char* buf_to   = turn ? g_buf2 : g_buf1;
            
            const int my_id = turn ? thread_id : (num_threads - thread_id - 1);
            
            const auto copied_idx = size_per_th * my_id;
            const auto copied_size = std::min(size_per_th, size - copied_idx);
            
            memcpy(&buf_to[copied_idx], &buf_from[copied_idx], copied_size);
            
            const auto t1 = get_cpu_clock();
            
            #pragma omp barrier
            
            const auto t2 = get_cpu_clock();
            
            write_acc.add(t1-t0);
            bar_acc.add(t2-t1);
        }
        
        print("write_result: {}\n", write_acc.summary());
        print("bar_result: {}\n", bar_acc.summary());
    }
    
    return 0;
}


