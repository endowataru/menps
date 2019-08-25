
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <time.h>
#include <sys/time.h>

double cur_time() {
  struct timeval tv[1];
  gettimeofday(tv, 0);
  return tv->tv_sec + tv->tv_usec * 1.0e-6;
}

extern "C"
int meomp_main(int argc, char** argv)
{
    using menps::mefdn::average_accumulator;
    using menps::mefdn::get_cpu_clock;
    using menps::mefdn::cpu_clock_t;
    using fmt::print;
    
    const size_t buf_size = atoi(argv[1]);
    //const size_t size_step = atoi(argv[2]);
    const int num_trials = atoi(argv[3]);
    auto* const buf = static_cast<unsigned char*>(meomp_malloc(buf_size));
    const int num_procs = meomp_get_num_procs();
    
    #pragma omp parallel
    {
        const int num_threads = omp_get_num_threads();
        const auto num_local_threads = meomp_get_num_local_threads();
        const int my_local_id = meomp_get_local_thread_num();
        
        const size_t size = buf_size;
        const size_t size_per_th =
            menps::mefdn::roundup_divide<size_t>(size, num_local_threads);
        const auto copied_idx = size_per_th * my_local_id;
        const auto copied_size = std::min(size_per_th, size - copied_idx);
        
        average_accumulator<cpu_clock_t> mem_acc;
        int sum = 0;

        double t_start = cur_time();
        
        for (size_t trial = 0; trial < num_trials; ++trial) {
            if (meomp_get_proc_num() == 0) {
                // write to buf
                for (size_t i = copied_idx; i < copied_idx+copied_size; ++i) {
                    buf[i] = static_cast<unsigned char>(trial*i);
                }
            }
            
            #pragma omp barrier
            
            if (meomp_get_proc_num() != 0) {
                for (size_t i = copied_idx; i < copied_idx+copied_size; ++i) {
                    sum += buf[i];
                }
            }
            
            #pragma omp barrier
        }
        
        const double t_end = cur_time();
        const double t = t_end - t_start;
        
        /*if (omp_get_thread_num() == 0) {
            print("- time: {} # [sec]\n", t);
            print("  transferred: {} # [bytes]\n", size*num_trials);
            print("  bandwidth: {} # [bytes/sec]\n", (size*num_trials)/t);
            print("  sum: {}\n", sum);
        }
        #pragma omp barrier*/
        for (int i = 0; i < num_threads; ++i) {
            if (omp_get_thread_num() == i) {
                print("- tid: {}\n", i);
                print("  time: {} # [sec]\n", t);
                print("  bandwidth: {} # [bytes/sec]\n", (size*num_trials)/t);
                print("  sum: {}\n", sum);
                print("  copied_idx: {}\n", copied_idx);
                print("  copied_size: {}\n", copied_size);
            }
            #pragma omp barrier
        }
    }
}

