
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/external/fmt.hpp>

extern "C"
int meomp_main(int argc, char** argv)
{
    using menps::mefdn::average_accumulator;
    using menps::mefdn::get_cpu_clock;
    using menps::mefdn::cpu_clock_t;
    using fmt::print;
    
    const size_t buf_size = atoi(argv[1]);
    const size_t size_step = atoi(argv[2]);
    const int num_trials = atoi(argv[3]);
    auto* const buf = static_cast<unsigned char*>(meomp_malloc(buf_size));
    const int num_procs = meomp_get_num_procs();
    
    #pragma omp parallel
    {
        const auto num_local_threads = meomp_get_num_local_threads();
        const int my_local_id = meomp_get_local_thread_num();
        
        for (int size = size_step; size <= buf_size; size += size_step)
        {
            const size_t size_per_th =
                menps::mefdn::roundup_divide<size_t>(size, num_local_threads);
            const auto copied_idx = size_per_th * my_local_id;
            const auto copied_size = std::min(size_per_th, size - copied_idx);
            
            average_accumulator<cpu_clock_t> mem_acc;
            //average_accumulator<cpu_clock_t> bar_acc;
            
            for (size_t t = 0; t < num_trials; ++t) {
                if (meomp_get_proc_num() == 0) {
                    const auto t0 = get_cpu_clock();
                    // write to buf
                    for (size_t i = copied_idx; i < copied_size; ++i) {
                        buf[i] = static_cast<unsigned char>(i);
                    }
                    const auto t1 = get_cpu_clock();
                    meomp_local_barrier();
                    const auto t2 = get_cpu_clock();
                    mem_acc.add(t2-t0);
                }
                
                #pragma omp barrier
                
                int sum = 0;
                if (meomp_get_proc_num() != 0) {
                    const auto t0 = get_cpu_clock();
                    // read from buf
                    for (size_t i = copied_idx; i < copied_size; ++i) {
                        buf[i] = static_cast<unsigned char>(i);
                    }
                    const auto t1 = get_cpu_clock();
                    meomp_local_barrier();
                    const auto t2 = get_cpu_clock();
                    mem_acc.add(t2-t0);
                }
                
                #pragma omp barrier
            }
            
            for (int i = 0; i < num_procs; ++i) {
                if (omp_get_thread_num() == i*num_local_threads) {
                    print("- {}: {}\n", i == 0 ? "write_time" : "read_time", mem_acc.summary());
                    //print("  bar_result: {}\n", bar_acc.summary());
                }
                #pragma omp barrier
            }
        }
    }
}

