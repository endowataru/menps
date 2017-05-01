
#include "bench_rma_msgrate.hpp"
#include <mgbase/external/fmt.hpp>
#include <fstream>
#include "basic_args.hpp"

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    const auto p = get_basic_args(argc, argv);
    
    const bool is_master =
        p.master_proc == static_cast<mgbase::int32_t>(mgcom::current_process_id());
    
    bench_rma_msgrate bench;
    
    bench.set_num_threads(p.num_threads);
    bench.set_num_startup_samples(p.num_startup_samples);
    bench.set_msg_size(p.msg_size);
    bench.set_target_proc(p.master_proc >= 0 ? static_cast<mgcom::process_id_t>(p.master_proc) : 0);
    
    bench.collective_setup();
    
    if (!is_master)
    {
        bench.start();
        
        mgbase::this_thread::sleep_for_ns(static_cast<mgbase::uint64_t>(p.duration * 1e9));
        
        bench.finish();
    }
    
    mgcom::collective::barrier();
    
    using std::fstream;
    using fmt::print;
    
    #define TAB "    "
    
    if (mgcom::current_process_id() == 0) {
        fstream ofs{p.output_file.c_str(), fstream::out | fstream::app};
        
        print(ofs, "- exp_type: bench-msgrate-read\n");
        print(ofs, "  number_of_processes: {}\n", mgcom::number_of_processes());
        print(ofs, "  number_of_threads: {}\n", p.num_threads);
        print(ofs, "  duration: {} # [sec]\n", p.duration);
        print(ofs, "  number_of_startup_samples: {}\n", p.num_startup_samples);
        print(ofs, "  master_proc: {}\n", p.master_proc);
        print(ofs, "  message_size: {} # [bytes]\n", p.msg_size);
        print(ofs, "  process_results:\n");
    }
    
    mgcom::collective::barrier();
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc)
    {
        if (!is_master && proc == mgcom::current_process_id())
        {
            mgbase::uint64_t count = 0;
            
            fstream ofs{p.output_file.c_str(), fstream::out | fstream::app};
            
            print(ofs, TAB "- process_id: {}\n", proc);
            print(ofs, TAB "  thread_results:\n");
            
            for (mgbase::uint32_t thread_id = 0; thread_id < p.num_threads; ++thread_id)
            {
                auto& info = bench.get_thread_info(thread_id);
                
                print(ofs, TAB TAB "- thread_id: {}\n", thread_id);
                print(ofs, TAB TAB "  count: {}\n", info.count);
                print(ofs, TAB TAB "  overhead: {} # [cycles]\n", info.overhead.summary());
                
                count += info.count;
            }
            
            print(ofs, TAB "total_count: {}\n", count);
            print(ofs, TAB "message_rate: {} [/sec]\n", count*1.0/p.duration);
        }
        
        mgcom::collective::barrier();
    }
    
    if (mgcom::current_process_id() == 0) {
        fstream ofs{p.output_file.c_str(), fstream::out | fstream::app};
        ofs << std::endl;
    }
    
    #undef TAB
    
    mgcom::finalize();
    
    return 0;
}

