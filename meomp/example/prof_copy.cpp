
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/arithmetic.hpp>

#define MAX_SIZE (1 << 20)

unsigned char g_buf[3][MAX_SIZE] MEOMP_GLOBAL_VAR;

extern "C"
int meomp_main(int argc, char** argv)
{
    using fmt::print;
    using menps::mefdn::average_accumulator;
    using menps::mefdn::get_cpu_clock;
    using menps::mefdn::cpu_clock_t;
    
    const auto num_trials = std::atoi(argv[1]);
    const auto max_size = std::atoi(argv[2]);
    const auto d = std::atoi(argv[3]);
    
    if (max_size > MAX_SIZE) {
        print("Too big. max={}\n", MAX_SIZE);
        return 1;
    }
    
    #pragma omp parallel
    {
        #pragma omp for
        for (int j = 0; j < max_size; ++j) {
            g_buf[0][j] = static_cast<unsigned char>(j);
            g_buf[1][j] = static_cast<unsigned char>(j+1);
            g_buf[2][j] = static_cast<unsigned char>(j+2);
        }
        
        const int num_threads = omp_get_num_threads();
        const int thread_id = omp_get_thread_num();
        
        for (int size = d; size <= max_size; size += d)
        {
            const auto size_per_th = menps::mefdn::roundup_divide(size, num_threads);
            
            average_accumulator<cpu_clock_t> write_acc;
            average_accumulator<cpu_clock_t> bar_acc;
            
            for (int i = 0; i < num_trials; ++i) {
                const auto t0 = get_cpu_clock();
                
                //const auto buf_from = g_buf[(i+1) % 3];
                //const auto buf_to   = g_buf[ i    % 3];
                const auto buf_from = g_buf[(i % 2) + 1];
                const auto buf_to   = g_buf[0];
                
                //const auto copied_idx = size_per_th * ((thread_id + i) % num_threads);
                const auto copied_idx = size_per_th * thread_id;
                const auto copied_size = std::min(size_per_th, size - copied_idx);
                
                memcpy(&buf_to[copied_idx], &buf_from[copied_idx], copied_size);
                
                const auto t1 = get_cpu_clock();
                
                #pragma omp barrier
                
                const auto t2 = get_cpu_clock();
                
                write_acc.add(t1-t0);
                bar_acc.add(t2-t1);
            }
            
            if (thread_id == 0) {
                print("- size: {} # [bytes]\n", size);
                print("  write_time: {} # [cycles]\n", write_acc.summary());
                print("  barrier_time: {} # [cycles]\n", bar_acc.summary());
            }
            
            #pragma omp barrier
        }
    }
    
    return 0;
}
