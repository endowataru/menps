
#define BENCH_USE_UCT 1
#include "bench.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <numeric>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <cmpth/prof/stat_accumulator.hpp>
#include <time.h>
#include <sys/time.h>
#include <vector>

constexpr bool g_is_debug = false;
constexpr bool g_is_result_checked = g_is_debug;
constexpr bool use_recv_comp = false;

//#define MY_ASSERT(x)    if (g_is_debug) { if (!(x)) { std::cerr << "Error: " << #x << std::endl; std::abort(); } }

struct accumulator_policy
{
    using sample_type = double;
    using real_type = double;
};

using accumulator_t = cmpth::stat_accumulator<accumulator_policy>;

inline double cur_time() {
    struct timeval tv[1];
    gettimeofday(tv, 0);
    return tv->tv_sec + tv->tv_usec * 1.0e-6;
}


using menps::fdn::size_t;
using menps::fdn::byte;
using menps::fdn::unique_ptr;
using menps::fdn::make_unique;
using menps::fdn::stopwatch;

using byte_remote_ptr_t = rma_itf_type::remote_ptr<byte>;
using byte_local_ptr_t = rma_itf_type::local_ptr<byte>;

int main(int argc, char** argv)
{
    com_itf_type com{&argc, &argv};
    g_com = &com;
    g_coll = &com.get_coll();
    g_rma = &com.get_rma();
    g_p2p = &com.get_p2p();

    {
    const std::size_t buf_size = 1ull << 25;//1ull << 9;
    auto buf1 = g_com->make_alltoall_buffer<byte>(buf_size);
    auto buf2 = g_com->make_alltoall_buffer<byte>(buf_size);

    for (std::size_t i = 0; i < buf_size; ++i) {
        *buf1.local(i) = static_cast<byte>(i+com.this_proc_id());
        *buf2.local(i) = static_cast<byte>(i+com.this_proc_id()+1);
    }

    //std::size_t size_points[] = { 1, 2, 8, 16, 32, 96, 128, 192, 256, 384, 512, 1024, 2048, 4096, 8<<10, 16<<10, 32<<10, 64<<10 };
    //std::size_t num_trials_list[] = { 100000, 100000, 100000, 100000,  };
    std::vector<std::size_t> size_points;
    size_points.push_back(1);
    for (std::size_t i = 0; i < 23; ++i) {
        size_points.push_back(2ull << i);
        size_points.push_back(3ull << i);
    }

    if (g_coll->this_proc_id() == 1) {
        //for (std::size_t x = 0; x < sizeof(size_points)/sizeof(size_points[0]); ++x) {
        //    const auto transfer_size = size_points[x];
            //const auto num_trials = num_trials_list[x];
        for (const auto transfer_size : size_points) {
            const auto base_trials = 600000;
            const auto num_trials = transfer_size < 8192 ? base_trials : static_cast<size_t>(300000.0 * 8192 / transfer_size);
            std::cerr << "Bench " << transfer_size << " [bytes] " << num_trials << " times ..." << std::endl;
            
            const auto start_time = cur_time();
            bool use_buf2 = false;
            
            for (std::size_t trial = 0; trial < num_trials; ++trial) { 
                const auto src = use_buf2 ? buf2.local(0) : buf1.local(0);
                const proc_id_type dest_proc = 0;
                const auto dest_ptr = buf1.remote(dest_proc, 0);
                g_rma->write(dest_proc, dest_ptr, src, transfer_size);
                use_buf2 = !use_buf2;
            }
            
            const auto end_time = cur_time();
            const auto t = end_time - start_time;
            //std::cout << transfer_size << " " << num_trials << " " << t << " " << (t/num_trials) << std::endl;
            
            std::stringstream ss;
            ss << "- size: " << transfer_size << " # [bytes]" << std::endl;
            ss << "  time: " << (t/2/num_trials) << " # [sec], one-way" << std::endl;
            ss << "  num_trials: " << num_trials << std::endl;
            ss << "  measurement_time: " << t << std::endl;

            const auto str = ss.str();
            std::cout << str;

            const char * file_path = getenv("BENCH_OUTPUT_PATH");
            if (file_path == NULL) { file_path = "output.yaml"; }
            std::ofstream ofs{file_path, std::ios::out | std::ios::app};   
            ofs << str;
        }
    }
    g_coll->barrier();

    }
    g_coll->barrier();

#if 0
    if (argc != 5) {
        std::cerr << argv[0] << " [mode] [block_size] [written_size] [num_trials]" << std::endl;
        return EXIT_FAILURE;
    }
    if (g_com->get_num_procs() != 2) {
        std::cerr << "Need to Run with 2 processes." << std::endl;
    }
    
    std::random_device rd;
    g_rand_gen = make_unique<std::mt19937>(rd());

    accumulator_t g_time;

    const auto mode = static_cast<diff_mode_t>(std::atoi(argv[1]));
    const auto block_size = static_cast<size_t>(std::atoi(argv[2]));
    const auto written_size = static_cast<size_t>(std::atoi(argv[3]));
    const auto num_trials = static_cast<size_t>(std::atoi(argv[4]));
    
    auto buf = g_com->make_alltoall_buffer<byte>(block_size);
    
    if (mode == diff_mode_t::write_contiguous) {
        const auto buf_lptr = buf.local(0);
        if (g_com->this_proc_id() == 0) {
            const proc_id_type remote_proc = 1;
            const auto buf_rptr = buf.remote(remote_proc, 0);
            for (size_t i = 0; i < num_trials; ++i) {
                make_dirty_data(buf_lptr, block_size, written_size);
                stopwatch sw;
                g_rma->write(remote_proc, buf_rptr, buf_lptr, block_size);
                if (use_recv_comp) {
                    g_rma->read(remote_proc, buf_rptr, buf_lptr, 1);
                }
                g_time.add(sw.elapsed());
                check_result(buf_lptr, block_size);
            }
        }
        else if (g_com->this_proc_id() == 1) {
            for (size_t i = 0; i < num_trials; ++i) {
                check_result(buf_lptr, block_size);
            }
        }
        g_coll->barrier();
    }
    else if (mode == diff_mode_t::write_discontiguous) {
        const auto buf_lptr = buf.local(0);
        if (g_com->this_proc_id() == 0) {
            const proc_id_type remote_proc = 1;
            const auto buf_rptr = buf.remote(remote_proc, 0);
            const auto twin = make_unique<byte []>(block_size);
            for (size_t i = 0; i < num_trials; ++i) {
                make_dirty_data(buf_lptr, block_size, written_size);
                stopwatch sw;
                write_discontiguous(remote_proc, buf_rptr, buf_lptr, twin.get(), block_size);
                if (use_recv_comp) {
                    g_rma->read(remote_proc, buf_rptr, buf_lptr, 1);
                }
                g_time.add(sw.elapsed());
                check_result(buf_lptr, block_size);
            }
        }
        else if (g_com->this_proc_id() == 1) {
            for (size_t i = 0; i < num_trials; ++i) {
                check_result(buf_lptr, block_size);
            }
        }
        g_coll->barrier();
    }
    else if (mode == diff_mode_t::send_diff) {
        const auto diff_buf_max_size = sizeof(diff_size_t)+(sizeof(diff_size_t)*2+1)*block_size;
        auto diff_buf = make_unique<byte []>(diff_buf_max_size);
        const auto buf_lptr = buf.local(0);
        if (g_com->this_proc_id() == 0) {
            const auto twin = make_unique<byte []>(block_size);
            for (size_t i = 0; i < num_trials; ++i) {
                make_dirty_data(buf_lptr, block_size, written_size);
                size_t diff_size = 0;
                stopwatch sw;
                pack_diff(buf_lptr, twin.get(), block_size, diff_buf.get(), &diff_size);
                g_p2p->send(1, 0, diff_buf.get(), diff_size);
                g_time.add(sw.elapsed());
                check_result(buf_lptr, block_size);
            }
        }
        else if (g_com->this_proc_id() == 1) {
            for (size_t i = 0; i < num_trials; ++i) {
                g_p2p->recv(0, 0, diff_buf.get(), diff_buf_max_size);
                unpack_diff(buf_lptr, block_size, diff_buf.get());
                check_result(buf_lptr, block_size);
            }
        }
        g_coll->barrier();
    }
    else {
        std::cerr << "Undefined diff mode: " << static_cast<int>(mode) << std::endl;
        return EXIT_FAILURE;
    }

    if (g_com->this_proc_id() == 0) {
        const char* mode_str = nullptr;
        if (mode == diff_mode_t::write_contiguous) { mode_str = "ContiguousWrite"; }
        if (mode == diff_mode_t::write_discontiguous) { mode_str = "DiscontiguousWrite"; }
        if (mode == diff_mode_t::send_diff) { mode_str = "PackDiff"; }
        std::stringstream ss;
        ss << "- mode: "  << mode_str << std::endl;
        ss << "  block_size: " << block_size << " # [bytes]" << std::endl;
        ss << "  written_size: " << written_size << " # [bytes]" << std::endl;
        ss << "  num_trials: " << num_trials << std::endl;
        ss << "  time: ";
        g_time.print_yaml(ss);
        ss << " # [cycles]" << std::endl;
        ss << std::endl;

        const auto str = ss.str();
        std::cout << str;
        
        const char * file_path = getenv("BENCH_OUTPUT_PATH");
        if (file_path == NULL) { file_path = "output.yaml"; }
        std::ofstream ofs{file_path, std::ios::out | std::ios::app};   
        ofs << str;
    }
#endif

    return EXIT_SUCCESS;
}

