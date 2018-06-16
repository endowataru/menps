
#include <menps/meomp.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/external/fmt.hpp>

#define MAX_SIZE (1 << 20)

unsigned char g_buf[MAX_SIZE] MEOMP_GLOBAL_VAR;

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
        
        for (int i = 0; i < num_trials; ++i) {
            const auto t0 = get_cpu_clock();
            
            #pragma omp for nowait
            for (int j = 0; j < size; ++j) {
                g_buf[j] = static_cast<unsigned char>(i);
            }
            
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

