
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/thread.hpp>
#include <mgbase/external/cmdline.hpp>
#include <iostream>
#include <vector>

mgbase::uint32_t g_num_threads;
mgbase::uint64_t g_num_trials;
mgbase::scoped_ptr<double [/*g_num_threads*/]> g_clocks;
mgcom::process_id_t g_root_proc;
mgcom::rma::remote_pointer<int> g_remote_ptr;

void start_bench_thread(const mgbase::uint32_t thread_id)
{
    const mgcom::rma::local_pointer<int> local_buf
        = mgcom::rma::allocate<int>();
    
    g_clocks[thread_id] = 0.0;
    
    for (mgbase::uint64_t i = 0; i < g_num_trials; i++) {
        mgbase::stopwatch sw;
        sw.start();
        
        const mgcom::process_id_t target_proc = g_root_proc; // FIXME
        
        mgcom::rma::remote_read_cb cb;
        mgcom::rma::remote_read_nb(
            cb
        ,   target_proc
        ,   g_remote_ptr
        ,   local_buf
        )
        .wait();
        
        g_clocks[thread_id] += sw.elapsed();
        
        if (*local_buf != 123)
            std::cout << "error!" << std::endl;
    }
}

void start_bench()
{
    mgcom::rma::local_pointer<int> local_ptr
        = mgcom::rma::allocate<int>();
    
    //const mgcom::process_id_t root_proc = 0;
    mgcom::collective::broadcast(g_root_proc, &local_ptr, 1);
    
    g_remote_ptr = mgcom::rma::use_remote_pointer(g_root_proc, local_ptr);
    
    if (mgcom::current_process_id() == g_root_proc)
        *local_ptr = 123;
    
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
    p.add<mgbase::uint64_t>("num_trials" , 'c', "number of trials" , false, 10000);
    
    p.parse_check(argc, argv);
    
    g_num_threads = p.get<mgbase::uint32_t>("num_threads");
    g_num_trials  = p.get<mgbase::uint64_t>("num_trials");
    
    g_root_proc = 0;
    
    g_clocks = new double[g_num_threads];
    
    start_bench();
    
    mgcom::collective::barrier();
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc) {
        if (proc == mgcom::current_process_id()) {
            for (mgbase::uint32_t thread_id = 0; thread_id < g_num_threads; ++thread_id)
                std::cout <<mgcom::current_process_id() << "\t" << thread_id << "\t"
                    << (g_clocks[thread_id] / g_num_trials) << std::endl; 
            
            //std::cout << mgcom::current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        }
        
        mgcom::collective::barrier();
    }
    
    mgcom::finalize();
    
    return 0;
}

