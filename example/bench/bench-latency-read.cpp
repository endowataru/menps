
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/thread.hpp>
#include <mgbase/threading/this_thread.hpp>
#include <mgbase/external/cmdline.hpp>
#include <mgbase/external/fmt.hpp>
#include <iostream>
#include <fstream>
#include <vector>

#include <mgbase/logging/queueing_logger.hpp>

#include <cmath>

#include <mgbase/profiling/average_accumulator.hpp>
#include <mgbase/profiling/stopwatch.hpp>

namespace /*unnamed*/ {

/*extern mgbase::average_accumulator<mgbase::cpu_clock_t> g_get_cycles;
extern mgbase::average_accumulator<mgbase::cpu_clock_t> g_poll_cycles;*/

bool g_finished = false;
mgbase::uint32_t g_num_threads;
double g_duration;
mgbase::uint64_t g_num_startup_samples;
mgbase::scoped_ptr<double [/*g_num_threads*/]> g_clocks_sum;
mgbase::scoped_ptr<double [/*g_num_threads*/]> g_clocks_sum_squared;
mgbase::scoped_ptr<mgcom::rma::remote_ptr<int> []> g_remote_ptrs;
mgbase::scoped_ptr<mgbase::uint64_t []> g_counts;
mgbase::scoped_ptr<double [/*g_num_threads*/]> g_total_time;

void record_sample(const mgbase::uint32_t thread_id, const double clocks)
{
    g_clocks_sum[thread_id] += clocks;
    g_clocks_sum_squared[thread_id] += clocks * clocks;
    
    ++g_counts[thread_id];
}

void start_bench_thread(const mgbase::uint32_t thread_id)
{
    const mgcom::rma::local_ptr<int> local_buf
        = mgcom::rma::allocate<int>();
    
    g_clocks_sum[thread_id] = 0.0;
    g_clocks_sum_squared[thread_id] = 0.0;
    g_counts[thread_id] = 0;
    
    mgbase::stopwatch total_sw;
    total_sw.start();
    
    for (mgbase::uint64_t i = 0; !g_finished; ++i)
    {
        *local_buf = 0;
        
        // Select the target process.
        mgcom::process_id_t target_proc;
        do {
            target_proc = static_cast<mgcom::process_id_t>(
                mgcom::number_of_processes() * ((1.0 - 1e-5) / RAND_MAX) * std::rand()
            );
        }
        while (target_proc == mgcom::current_process_id());
        
        mgbase::stopwatch sw;
        sw.start();
        
        mgcom::rma::read(
            target_proc
        ,   g_remote_ptrs[target_proc]
        ,   local_buf
        ,   1
        );
        
        if (i > g_num_startup_samples)
            record_sample(thread_id, sw.elapsed());
        
        if (*local_buf != target_proc * 100)
            std::cout << "error!" << std::endl;
    }
    
    g_total_time[thread_id] = total_sw.elapsed();
}

void start_bench()
{
    // Set up variables.
    
    mgcom::rma::local_ptr<int> local_ptr
        = mgcom::rma::allocate<int>();
    
    mgbase::scoped_ptr<mgcom::rma::local_ptr<int> []> local_ptrs(
        new mgcom::rma::local_ptr<int>[mgcom::number_of_processes()]
    );
    
    mgcom::collective::allgather(&local_ptr, &local_ptrs[0], 1);
    
    g_remote_ptrs = new mgcom::rma::remote_ptr<int>[mgcom::number_of_processes()];
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
        g_remote_ptrs[proc] = mgcom::rma::use_remote_ptr(proc, local_ptrs[proc]);
    
    *local_ptr = mgcom::current_process_id() * 100;
    
    mgcom::collective::barrier();
    
    // Create threads.
    mgbase::scoped_ptr<mgbase::thread []> ths(new mgbase::thread[g_num_threads]);
    
    for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
        ths[thread_id] =
            mgbase::thread(
                mgbase::bind1st_of_1(&start_bench_thread, thread_id)
            );
    
    // Sleep awhile.
    mgbase::this_thread::sleep_for_ns(static_cast<mgbase::uint64_t>(g_duration * 1e9));
    g_finished = true;
    
    // Join all.
    for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
        ths[thread_id].join();
}

} // unnamed namespace

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    cmdline::parser p;
    p.add<mgbase::uint32_t>("num_threads", 't', "number of threads", false, 1);
    p.add<double>("duration", 'd', "duration [sec]", false, 3.0);
    p.add<mgbase::uint64_t>("num_startup_samples" , 's', "number of initially omitted samples" , false, 1000);
    p.add<std::string>("output_file", 'o', "path to the output file", false, "output.yaml");
    
    p.parse_check(argc, argv);
    
    g_num_threads = p.get<mgbase::uint32_t>("num_threads");
    g_duration = p.get<double>("duration");
    g_num_startup_samples = p.get<mgbase::uint64_t>("num_startup_samples");
    const std::string output_file = p.get<std::string>("output_file");
    
    g_clocks_sum = new double[g_num_threads];
    g_clocks_sum_squared = new double[g_num_threads];
    g_counts = new mgbase::uint64_t[g_num_threads];
    g_total_time = new double[g_num_threads];
    
    start_bench();
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        std::fstream ofs(output_file.c_str(), std::fstream::out | std::fstream::app);
        
        fmt::print(ofs, "- exp_type: bench-latency-read\n");
        fmt::print(ofs, "  number_of_processes: {}\n", mgcom::number_of_processes());
        fmt::print(ofs, "  number_of_threads: {}\n", g_num_threads);
        fmt::print(ofs, "  duration: {} # [sec]\n", g_duration);
        fmt::print(ofs, "  number_of_startup_samples: {}\n", g_num_startup_samples);
        fmt::print(ofs, "  process_results:\n");
    }
    
    mgcom::collective::barrier();
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
    {
        if (proc == mgcom::current_process_id())
        {
            std::fstream ofs(output_file.c_str(), std::fstream::out | std::fstream::app);
            
            fmt::print(ofs, "    - process_id: {}\n", proc);
            fmt::print(ofs, "      thread_results:\n");
            
            for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
            {
                const mgbase::uint64_t count = g_counts[thread_id];
                const double latency_average = g_clocks_sum[thread_id] / count;
                const double latency_stddev
                    = std::sqrt(g_clocks_sum_squared[thread_id] / count - latency_average * latency_average);
                
                fmt::print(ofs, "        - thread_id: {}\n", thread_id);
                fmt::print(ofs, "          latency: {{ average: {}, stddev: {} }} # [cycles]\n",
                    latency_average, latency_stddev);
                fmt::print(ofs, "          count: {}\n", count);
                fmt::print(ofs, "          time: {} # [cycles]\n", g_total_time[thread_id]);
            }
            
        }
        
        mgcom::collective::barrier();
    }
    
    if (mgcom::current_process_id() == 0) {
        std::fstream ofs(output_file.c_str(), std::fstream::out | std::fstream::app);
        ofs << std::endl;
    }
    
    mgcom::finalize();
    
    if (mgcom::current_process_id() == 0) // TODO: finalized
    {
        /*std::cout << g_get_cycles.summary() << std::endl;
        std::cout << g_poll_cycles.summary() << std::endl;*/
        
        const std::string header = fmt::format("proc:{}\t", mgcom::current_process_id());
        mgbase::queueing_logger::show(header.c_str());
    }
    
    return 0;
}

