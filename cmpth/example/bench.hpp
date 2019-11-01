
#pragma once

#include <cmpth/sct/def_sct_itf.hpp>
#include <cmpth/wrap/mth_itf.hpp>
#include <cmpth/wrap/ctmth_itf.hpp>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <sys/time.h>

inline double cur_time() {
    struct timeval tv[1];
    gettimeofday(tv, 0);
    return tv->tv_sec + tv->tv_usec * 1.0e-6;
}

template <template <typename> class Bench, typename UltItf>
inline void exec_bench_impl(const char* const ult_itf_name, const int argc, char** const argv)
{
    typename UltItf::initializer sched_init CMPTH_MAYBE_UNUSED;

    Bench<UltItf> b{argc, argv};
    double t0 = cur_time();
    auto ret = b();
    double t1 = cur_time();

    std::ostringstream ss;
    ss << "- name: " << b.name() << std::endl;
    const auto correct = b.is_correct(ret);
    ss << "  correct: " << correct << std::endl;
    if (correct) {
        ss << "  time: " << (t1-t0) << " # [sec]" << std::endl;
        ss << "  num_workers: " << UltItf::get_num_workers() << std::endl;
        ss << "  ult_itf: " << ult_itf_name << std::endl;
        const auto conf = b.get_config();
        for (const auto& c: conf) {
            using std::get;
            ss << "  " << get<0>(c) << ": " << get<1>(c) << std::endl;
        }
        ss << std::endl;
    }

    const auto str = ss.str();
    std::cout << str;
 
    const char * file_path = getenv("BENCH_OUTPUT_PATH");
    if (file_path == NULL) { file_path = "output.yaml"; }
    std::ofstream ofs{file_path, std::ios::out | std::ios::app};   
    ofs << str;
}

template <template <typename> class Bench>
inline int exec_bench(const int argc, char** const argv)
{
    const char env_name[] = "BENCH_ULT_ITF";
    const auto* const ult_itf_name = std::getenv(env_name);
    if (ult_itf_name == nullptr) {
        std::cerr << "Specify env " << env_name << "." << std::endl;
        return 1;
    }
    if (std::strcmp(ult_itf_name, "SCT") == 0) {
        exec_bench_impl<Bench, cmpth::def_sct_itf>(ult_itf_name, argc, argv);
        return 0;
    }
    if (std::strcmp(ult_itf_name, "MTH") == 0) {
        exec_bench_impl<Bench, cmpth::mth_itf>(ult_itf_name, argc, argv);
        return 0;
    }
    if (std::strcmp(ult_itf_name, "CTMTH") == 0) {
        exec_bench_impl<Bench, cmpth::ctmth_itf>(ult_itf_name, argc, argv);
        return 0;
    }
    std::cerr << "Not found ULT interface: " << ult_itf_name << std::endl;
    return 1;
}

