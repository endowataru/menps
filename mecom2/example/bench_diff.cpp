
#include "bench.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <random>
#include <numeric>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <cmpth/prof/stat_accumulator.hpp>

constexpr bool g_is_debug = false;
constexpr bool g_is_result_checked = g_is_debug;
constexpr bool use_recv_comp = false;

#define MY_ASSERT(x)    if (g_is_debug) { if (!(x)) { std::cerr << "Error: " << #x << std::endl; std::abort(); } }

struct accumulator_policy
{
    using sample_type = menps::fdn::cpu_clock_t;
    using real_type = double;
};

using accumulator_t = cmpth::stat_accumulator<accumulator_policy>;

using menps::fdn::size_t;
using menps::fdn::byte;
using menps::fdn::unique_ptr;
using menps::fdn::make_unique;
using menps::fdn::stopwatch;

enum class diff_mode_t {
    write_contiguous = 1
,   write_discontiguous
,   send_diff
};

unique_ptr<std::mt19937> g_rand_gen;

inline void make_dirty_data(byte* const buf, const size_t block_size, const size_t written_size)
{
    std::vector<size_t> indexes(block_size);
    std::iota(std::begin(indexes), std::end(indexes), 0);
    std::shuffle(std::begin(indexes), std::end(indexes), *g_rand_gen);

    for (size_t i = 0; i < written_size; ++i) {
        const auto buf_idx = indexes[i];
        if (g_is_debug) {
            std::cout << "Modify [" << buf_idx << "]." << std::endl;
        }
        buf[buf_idx] = static_cast<byte>(static_cast<int>(buf[buf_idx]) + 1);
    }
}

using byte_remote_ptr_t = rma_itf_type::remote_ptr<byte>;
using byte_local_ptr_t = rma_itf_type::local_ptr<byte>;

inline void write_discontiguous(
    proc_id_type proc, byte_remote_ptr_t rptr, byte_local_ptr_t lptr, byte* twin, size_t block_size)
{
    std::vector<rma_itf_type::request_type> reqs;
    for (size_t i = 0; i < block_size; /* advanced inside the loop */) {
        // Scan clean bytes.
        for ( ; i < block_size && lptr[i] == twin[i]; ++i) { }
        if (!(i < block_size)) { break; }
        
        // Scan dirty bytes.
        size_t j = i+1;
        for ( ; j < block_size; ++j) {
            if (lptr[j] == twin[j]) { break; }
        }
        
        if (g_is_debug) {
            std::cout << "Writing [" << i << ", " << j << ")." << std::endl;
        }
        rma_itf_type::request_type req = rma_itf_type::request_type();
        g_rma->untyped_write_nb(proc, rptr+i, lptr+i, j-i, &req);
        reqs.push_back(std::move(req));

        std::memcpy(twin+i, lptr+i, j-i);
        i = j;
    }
    for (auto& req: reqs) {
        g_rma->wait(&req);
    }
}

using diff_size_t = std::uint32_t;

inline void pack_diff(
    byte* lptr, byte* twin, size_t block_size,
    byte* out_diff_buf, size_t* out_diff_size
) {
    diff_size_t out_diff_idx = 0;
    out_diff_idx += static_cast<diff_size_t>(sizeof(diff_size_t));
    for (size_t i = 0; i < block_size; /* advanced inside the loop */) {
        // Scan clean bytes.
        for ( ; i < block_size && lptr[i] == twin[i]; ++i) { }
        if (!(i < block_size)) { break; }
        
        // Scan dirty bytes.
        size_t j = i+1;
        for ( ; j < block_size; ++j) {
            if (lptr[j] == twin[j]) { break; }
        }
        
        if (g_is_debug) {
            std::cout << "Packing [" << i << ", " << j << ")." << std::endl;
        }

        const auto write_index = static_cast<diff_size_t>(i);
        const auto write_size = static_cast<diff_size_t>(j-i);

        // Note: unaligned access
        memcpy(&out_diff_buf[out_diff_idx], &write_index, sizeof(diff_size_t));
        out_diff_idx += static_cast<diff_size_t>(sizeof(diff_size_t));
        memcpy(&out_diff_buf[out_diff_idx], &write_size, sizeof(diff_size_t));
        out_diff_idx += static_cast<diff_size_t>(sizeof(diff_size_t));
        
        memcpy(&out_diff_buf[out_diff_idx], &lptr[i], write_size);
        out_diff_idx += write_size;

        std::memcpy(twin+i, lptr+i, j-i);
        i = j;
    }
    memcpy(&out_diff_buf[0], &out_diff_idx, sizeof(diff_size_t));
    *out_diff_size = out_diff_idx;
    if (g_is_debug) { std::cout << "Total diff size when packed: " << out_diff_idx << std::endl; }
}

inline void unpack_diff(byte* lptr, size_t block_size, byte* diff_buf) {
    size_t i = 0;
    diff_size_t diff_size = 0;
    memcpy(&diff_size, diff_buf, sizeof(diff_size_t));
    i += sizeof(diff_size_t);
    if (g_is_debug) { std::cout << "Total diff size when unpacked: " << diff_size << std::endl; }

    for ( ; i < diff_size; /* advanced inside the loop */) {
        diff_size_t write_index = 0, write_size = 0;
        memcpy(&write_index, &diff_buf[i], sizeof(diff_size_t));
        i += sizeof(diff_size_t);
        memcpy(&write_size, &diff_buf[i], sizeof(diff_size_t));
        i += sizeof(diff_size_t);

        MY_ASSERT(write_index < block_size);
        if (g_is_debug) {
            std::cout << "Unpacking [" << write_index << ", " << (write_index+write_size) << ")." << std::endl;
        }
        
        memcpy(&lptr[write_index], &diff_buf[i], write_size);
        i += write_size;
    }
}

inline void check_result(byte* lptr, size_t block_size)
{
    if (g_is_result_checked) {
        g_coll->barrier();
        size_t sum = 0;
        for (int i = 0; i < block_size; ++i) {
            sum += static_cast<size_t>(lptr[i]);
        }
        const auto sums = make_unique<size_t []>(g_coll->get_num_procs());
        g_coll->allgather(&sum, sums.get(), 1);
        for (size_t proc = 0; proc < g_coll->get_num_procs(); ++proc) {
            if (sum != sums[proc]) {
                std::cout << "Check failed on proc " << g_coll->this_proc_id() << ": " << sum << " " << sums[proc] << std::endl;
                std::abort();
            }
        }
    }
}

int main(int argc, char** argv)
{
    com_itf_type com{&argc, &argv};
    g_com = &com;
    g_coll = &com.get_coll();
    g_rma = &com.get_rma();
    g_p2p = &com.get_p2p();

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

    return EXIT_SUCCESS;
}

