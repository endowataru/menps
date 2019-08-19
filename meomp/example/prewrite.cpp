
#include <menps/meomp.hpp>
#include <omp.h>
#include <menps/mefdn/profiling/average_accumulator.hpp>
#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <fstream>
#include <iostream>

extern "C"
int meomp_main(int argc, char** argv)
{
    using menps::mefdn::average_accumulator;
    using menps::mefdn::get_cpu_clock;
    using menps::mefdn::cpu_clock_t;
    using fmt::print;
    
    const size_t buf_size = atoi(argv[1]);
    const size_t size_step = atoi(argv[2]);
    const size_t prewrite_step = atoi(argv[3]);
    const int n_prewrites = atoi(argv[4]);
    const int num_trials = atoi(argv[5]);

    auto* const buf = static_cast<unsigned char*>(meomp_malloc(buf_size));

    #pragma omp parallel
    {
        const int num_procs = meomp_get_num_procs();
        const auto num_local_threads = meomp_get_num_local_threads();
        const int ltid = meomp_get_local_thread_num();

        average_accumulator<cpu_clock_t> mem_acc;
        average_accumulator<cpu_clock_t> bar_acc;
        average_accumulator<cpu_clock_t> total_acc;
        
        const auto t_start = get_cpu_clock();
        for (int trial = 0; trial < num_trials; ++trial) {
            for (int turn = 0; turn < num_procs; ++turn) {
                for (size_t i = 0; i < buf_size; i += size_step) {
                    const auto t0 = get_cpu_clock();
                    if (meomp_get_proc_num() == turn) {
                        if (ltid == 0) {
                            for (size_t j = i; j < std::min(i + size_step, buf_size); ++j) {
                                buf[j] = static_cast<unsigned char>(j+trial+turn);
                            }
                        }
                        else if (ltid <= n_prewrites) {
                            if (i % prewrite_step == 0) {
                                // pre-write
                                const size_t pos = i + ltid * prewrite_step;
                                if (pos < buf_size) {
                                    buf[pos] = static_cast<unsigned char>(pos+trial+turn);
                                }
                            }
                        }
                    }
                    const auto t1 = get_cpu_clock();
                    #pragma omp barrier
                    const auto t2 = get_cpu_clock();
                    mem_acc.add(t1-t0);
                    bar_acc.add(t2-t1);
                }
            }
        }
        const auto t_end = get_cpu_clock();
        total_acc.add(t_end - t_start);
        
        const int num_threads = omp_get_num_threads();
        const char * file_path = getenv("BENCH_OUTPUT_PATH");
        if (file_path == NULL) { file_path = "output.yaml"; }
        
        for (int i = 0; i < num_threads; ++i) {
            const int tid = omp_get_thread_num();
            if (i == tid) {
                fmt::memory_buffer w;
                if (tid == 0) {
                    format_to(w, "- buf_size : {}\n", buf_size);
                    format_to(w, "  size_step : {}\n", size_step);
                    format_to(w, "  prewrite_step : {}\n", prewrite_step);
                    format_to(w, "  n_prewrites : {}\n", n_prewrites);
                    format_to(w, "  num_trials : {}\n", num_trials);
                }
                format_to(w, "  - thread_num: {}\n", omp_get_thread_num());
                format_to(w, "    write_time: {}\n", mem_acc.summary());
                format_to(w, "    barrier_time: {}\n", bar_acc.summary());
                format_to(w, "    total_time: {}\n", total_acc.summary());

                const auto str = to_string(w);
                std::cout << str << std::flush;
                std::ofstream ofs(file_path, std::ios_base::app);
                ofs << str;
            }
            #pragma omp barrier
        }
    }
}

