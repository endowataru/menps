
#pragma once

#include <cmpth/sct/def_sct_itf.hpp>
#include <cmpth/wrap/mth_itf.hpp>
#include <cmpth/wrap/ctmth_itf.hpp>
#ifdef CMPTH_ENABLE_ABT
    #include <cmpth/wrap/abt_itf.hpp>
#endif
#include <cmpth/wrap/dummy_ult_itf.hpp>
#include <cmpth/wrap/kth_itf.hpp>
#include <cmpth/ult_ext_itf.hpp>
#include <cmpth/prof.hpp>
#include <cmpth/prof/prof_tag.hpp>
#include CMPTH_PROF_HEADER_DUMMY
//#include CMPTH_PROF_HEADER_TRACE
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

#define BENCH_PROF_KINDS(x) \
    x(bench_event)

struct prof_barrier_aspect {
    CMPTH_DEFINE_PROF_ASPECT_POLICY(BENCH_PROF_KINDS)
    
    static constexpr std::size_t get_default_mlog_size() noexcept {
        return 1ull << 20;
    }
};

template <typename UltItf>
struct bench_policy {
    using ult_itf_type = UltItf;
    using prof_aspect_type =
        typename UltItf::template prof_aspect_t<
            cmpth::prof_tag::DUMMY,
            prof_barrier_aspect>;
};

template <template <typename> class Bench, cmpth::ult_tag_t UltTag>
inline void exec_bench_impl(const char* const ult_itf_name, const int argc, char** const argv)
{
    using ult_itf_type = cmpth::get_ult_ext_itf_t<UltTag, cmpth::sync_tag_t::MCS>;
    typename ult_itf_type::initializer sched_init{argc, argv};

    using bench_policy_type = bench_policy<ult_itf_type>;
    Bench<bench_policy_type> b{argc, argv};
    double t0 = cur_time();
    auto ret = b();
    double t1 = cur_time();

    std::ostringstream ss;
    ss << "- name: " << b.name() << std::endl;
    const auto correct = b.is_correct(ret);
    ss << "  correct: " << correct << std::endl;
    if (correct) {
        const auto t_intvl = t1-t0;
        const auto num_ops = b.num_total_ops();
        ss << "  time: " << t_intvl << " # [sec]" << std::endl;
        ss << "  num_ops: " << num_ops << " # [ops]" << std::endl;
        ss << "  ops_per_sec: " << (num_ops/t_intvl) << " # [ops/sec]" << std::endl;
        ss << "  num_workers: " << ult_itf_type::get_num_workers() << std::endl;
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
    
    bench_policy_type::prof_aspect_type::print_all("cmpth", 0);
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
        exec_bench_impl<Bench, cmpth::ult_tag_t::SCT>(ult_itf_name, argc, argv);
        return 0;
    }
    if (std::strcmp(ult_itf_name, "MTH") == 0) {
        exec_bench_impl<Bench, cmpth::ult_tag_t::MTH>(ult_itf_name, argc, argv);
        return 0;
    }
    if (std::strcmp(ult_itf_name, "CTMTH") == 0) {
        exec_bench_impl<Bench, cmpth::ult_tag_t::CTMTH>(ult_itf_name, argc, argv);
        return 0;
    }
    #if defined(CMPTH_ENABLE_ABT) && !defined(BENCH_AVOID_ABT)
    if (std::strcmp(ult_itf_name, "ABT") == 0) {
        exec_bench_impl<Bench, cmpth::ult_tag_t::ABT>(ult_itf_name, argc, argv);
        return 0;
    }
    #endif
    #if !defined(BENCH_AVOID_DUMMY)
    if (std::strcmp(ult_itf_name, "DUMMY") == 0) {
        exec_bench_impl<Bench, cmpth::ult_tag_t::DUMMY>(ult_itf_name, argc, argv);
        return 0;
    }
    #endif
    #if !defined(BENCH_AVOID_KTH)
    if (std::strcmp(ult_itf_name, "KTH") == 0) {
        exec_bench_impl<Bench, cmpth::ult_tag_t::KTH>(ult_itf_name, argc, argv);
        return 0;
    }
    #endif
    std::cerr << "Not found ULT interface: " << ult_itf_name << std::endl;
    return 1;
}

