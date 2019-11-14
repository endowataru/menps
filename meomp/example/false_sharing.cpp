
#include <iostream>
#include <sstream>
#include <fstream>
#include <cstdlib>
#include <time.h>
#include <sys/time.h>
#include <cassert>

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifdef MEOMP_ENABLED
    // meomp
    #include <menps/meomp.hpp>
#else
    // normal
    #define meomp_main  main
    static inline int meomp_get_num_procs(void) { return 1; }
    static inline int meomp_get_num_local_threads(void) {
        #ifdef _OPENMP
        return omp_get_num_threads();
        #else
        return 1;
        #endif
    }
    static inline void * meomp_malloc(size_t size) { return malloc(size); }
#endif

inline double cur_time() {
    struct timeval tv[1];
    gettimeofday(tv, 0);
    return tv->tv_sec + tv->tv_usec * 1.0e-6;
}

extern "C"
int meomp_main(int argc, char** argv)
{
    using byte = unsigned char;
    
    const auto total_size = static_cast<std::size_t>(std::atoi(argv[1]));
    const auto size_per_th = static_cast<std::size_t>(std::atoi(argv[2]));
    const auto num_repeats = static_cast<std::size_t>(std::atoi(argv[3]));
    const auto do_verify = static_cast<std::size_t>(std::atoi(argv[4]));
    
    constexpr std::size_t page_size = 32<<10;
    byte* arr = static_cast<byte*>(meomp_malloc(total_size + page_size));
    arr += (page_size - reinterpret_cast<std::uintptr_t>(arr) % page_size);
    assert(reinterpret_cast<std::uintptr_t>(arr) % page_size == 0);
    //std::cout << reinterpret_cast<std::uintptr_t>(arr) << std::endl;

    const auto t0 = cur_time();
    #pragma omp parallel
    {
        for (std::size_t r = 0; r < num_repeats; ++r) {
            const auto tid = omp_get_thread_num();
            const auto nthreads = omp_get_num_threads();
            for (std::size_t i = tid*size_per_th; i < total_size; i += size_per_th*nthreads) {
                for (std::size_t j = i; j < std::min(i+size_per_th, total_size); ++j) {
                    ++arr[j];
                }
            }
            #pragma omp barrier
        }
    }
    const auto t1 = cur_time();
    
    #pragma omp master
    {
        size_t num_wrong = 0;
        if (do_verify) {
            for (std::size_t i = 0; i < total_size; ++i) {
                num_wrong += (arr[i] != static_cast<byte>(num_repeats));
            }
        }

        std::stringstream ss;
        ss << "- bench_name: false_sharing" << std::endl;
        ss << "  total_size: " << total_size << " # [bytes]" << std::endl;
        ss << "  size_per_th: " << size_per_th << " # [bytes]" << std::endl;
        ss << "  num_repeats: " << num_repeats << std::endl;
        if (do_verify) {
            ss << "  correct: " << (num_wrong == 0) << std::endl;
            ss << "  num_wrong: " << num_wrong << std::endl;
        }
        ss << "  num_threads: " << omp_get_max_threads() << std::endl;
        ss << "  num_procs: " << meomp_get_num_procs() << std::endl;
        ss << "  num_local_threads: " << meomp_get_num_local_threads() << std::endl;
        const auto total_time = t1 - t0;
        ss << "  total_time: " << total_time << " # [sec]" << std::endl;
        const auto latency = total_time / num_repeats;
        ss << "  latency: " << latency  << " # [sec]" << std::endl;
        ss << "  bandwidth: " << (total_size/latency)  << " # [bytes/sec]" << std::endl;

        const auto str = ss.str();
        std::cout << str;

        const char * file_path = getenv("BENCH_OUTPUT_PATH");
        if (file_path == NULL) { file_path = "output.yaml"; }
        std::ofstream ofs{file_path, std::ios::out | std::ios::app};
        ofs << str;
    }
    
    #pragma omp barrier
    
    return 0;
}
    

