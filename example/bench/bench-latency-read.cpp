
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/thread.hpp>
#include <mgbase/external/cmdline.hpp>
#include <iostream>
#include <vector>

#include <cmath>

mgbase::uint32_t g_num_threads;
mgbase::uint64_t g_num_trials;
mgbase::uint64_t g_num_startup_samples;
mgbase::scoped_ptr<double [/*g_num_threads*/]> g_clocks_sum;
mgbase::scoped_ptr<double [/*g_num_threads*/]> g_clocks_sum_squared;
mgcom::process_id_t g_root_proc;
mgbase::scoped_ptr<mgcom::rma::remote_pointer<int> []> g_remote_ptrs;

void record_sample(const mgbase::uint32_t thread_id, const double clocks)
{
    g_clocks_sum[thread_id] += clocks;
    g_clocks_sum_squared[thread_id] += clocks * clocks;
}

void start_bench_thread(const mgbase::uint32_t thread_id)
{
    const mgcom::rma::local_pointer<int> local_buf
        = mgcom::rma::allocate<int>();
    
    g_clocks_sum[thread_id] = 0.0;
    g_clocks_sum_squared[thread_id] = 0.0;
    
    const mgbase::uint64_t num_total_trials = g_num_startup_samples + g_num_trials;
    
    for (mgbase::uint64_t i = 0; i < num_total_trials; i++) {
        *local_buf = 0;
        
        mgcom::process_id_t target_proc;
        do {
            target_proc = static_cast<mgcom::process_id_t>(
                mgcom::number_of_processes() * ((1.0 - 1e-5) / RAND_MAX) * std::rand()
            );
        }
        while (target_proc == mgcom::current_process_id());
        //const mgcom::process_id_t target_proc = 1 - g_root_proc;
        //const mgcom::process_id_t target_proc = g_root_proc; // FIXME
        
        mgbase::stopwatch sw;
        sw.start();
        
        mgbase::atomic<bool> flag = MGBASE_ATOMIC_VAR_INIT(false);
        mgcom::rma::remote_read_async(
            target_proc
        ,   g_remote_ptrs[target_proc]
        ,   local_buf
        ,   1
        ,   mgbase::make_operation_store_release(&flag, true)
        );
        
        while (!flag.load(mgbase::memory_order_acquire)) { }
        
        if (i > g_num_startup_samples)
            record_sample(thread_id, sw.elapsed());
        
        if (*local_buf != target_proc * 100)
            std::cout << "error!" << std::endl;
    }
}

void start_bench()
{
    mgcom::rma::local_pointer<int> local_ptr
        = mgcom::rma::allocate<int>();
    
    mgbase::scoped_ptr<mgcom::rma::local_pointer<int> []> local_ptrs(
        new mgcom::rma::local_pointer<int>[mgcom::number_of_processes()]
    );
    
    //const mgcom::process_id_t root_proc = 0;
    //mgcom::collective::broadcast(g_root_proc, &local_ptr, 1);
    mgcom::collective::allgather(&local_ptr, &local_ptrs[0], 1);
    
    g_remote_ptrs = new mgcom::rma::remote_pointer<int>[mgcom::number_of_processes()];
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
        g_remote_ptrs[proc] = mgcom::rma::use_remote_pointer(proc, local_ptrs[proc]);
    
    //if (mgcom::current_process_id() == g_root_proc)
    *local_ptr = mgcom::current_process_id() * 100;
    
    mgcom::collective::barrier();
    
    std::vector<mgbase::thread> ths;
    
    for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
        ths.push_back(
            mgbase::thread(
                mgbase::bind1st_of_1(&start_bench_thread, thread_id)
            )
        );
    
    for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
        ths[thread_id].join();
    
}

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    cmdline::parser p;
    p.add<mgbase::uint32_t>("num_threads", 'p', "number of threads", false, 1);
    p.add<mgbase::uint64_t>("num_trials" , 'c', "number of trials" , false, 1000000);
    p.add<mgbase::uint64_t>("num_startup_samples" , 's', "number of initially omitted samples" , false, 1000);
    
    p.parse_check(argc, argv);
    
    g_num_threads = p.get<mgbase::uint32_t>("num_threads");
    g_num_trials  = p.get<mgbase::uint64_t>("num_trials");
    g_num_startup_samples = p.get<mgbase::uint64_t>("num_startup_samples");
    
    g_root_proc = 0;
    
    g_clocks_sum = new double[g_num_threads];
    g_clocks_sum_squared = new double[g_num_threads];
    
    start_bench();
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        fmt::print("- exp_type: bench-latency-read\n");
        fmt::print("  number_of_processes: {}\n", mgcom::number_of_processes());
        fmt::print("  number_of_threads: {}\n", g_num_threads);
        fmt::print("  number_of_trials: {}\n", g_num_trials);
        fmt::print("  number_of_startup_samples: {}\n", g_num_startup_samples);
        fmt::print("  process_results:\n");
        std::cout << std::flush;
    }
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
    {
        if (proc == mgcom::current_process_id())
        {
            fmt::print("    - process_id : {}\n", proc);
            fmt::print("      thread_results:\n");
            
            for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
            {
                const double latency_average = g_clocks_sum[thread_id] / g_num_trials;
                const double latency_stddev
                    = std::sqrt(g_clocks_sum_squared[thread_id] / g_num_trials - latency_average * latency_average);
                
                fmt::print("        - thread_id: {}\n", thread_id);
                fmt::print("          latency: {{ average: {}, stddev: {} }} # [clocks]\n",
                    latency_average, latency_stddev);
            }
            
            std::cout << std::flush;
        }
        
        mgcom::collective::barrier();
    }
    
    if (mgcom::current_process_id() == 0) {
        std::cout << std::endl;
    }
    
    mgcom::finalize();
    
    return 0;
}

