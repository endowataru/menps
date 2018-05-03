 
#include <menps/meomp.hpp>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/external/fmt.hpp>

extern "C"
int meomp_main(int argc, char** argv)
{
    using fmt::print;
    using menps::mefdn::average_accumulator;
    using menps::mefdn::get_cpu_clock;
    using menps::mefdn::cpu_clock_t;
    
    const auto num_trials = std::atoi(argv[1]);
    
    #pragma omp parallel
    {
        average_accumulator<cpu_clock_t> acc;
        
        for (int i = 0; i < num_trials; ++i) {
            const auto t0 = get_cpu_clock();
            
            #pragma omp barrier
            
            const auto t1 = get_cpu_clock();
            
            acc.add(t1-t0);
        }
        
        print("result: {}\n", acc.summary());
    }
    
    return 0;
}
 
 
