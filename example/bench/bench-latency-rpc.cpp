
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

struct bench_handler {
    static const mgcom::rpc::handler_id_t handler_id = 1;
    
    struct request_type { };
    typedef void reply_type;
    
    template <typename ServerCtx>
    typename ServerCtx::return_type operator() (ServerCtx& sc) const
    {
        return sc.make_reply();
    }
};

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    const mgcom::process_id_t root_proc = 0;
    const int number_of_trials = atoi(argv[1]);
    
    mgcom::rpc::register_handler2(mgcom::rpc::requester::get_instance(), bench_handler{});
    
    mgcom::collective::barrier();
    
    double clocks = 0;
    
    if (mgcom::current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mgbase::stopwatch sw;
            sw.start();
            
            mgcom::rpc::call2<bench_handler>(
                mgcom::rpc::requester::get_instance()
            ,   root_proc
            ,   bench_handler::request_type()
            );
            
            clocks += sw.elapsed();
        }
    }
    
    mgcom::collective::barrier();
    
    for (mgcom::process_id_t proc = 0; proc < mgcom::number_of_processes(); ++proc) {
        if (proc == mgcom::current_process_id())
            std::cout << mgcom::current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        
        mgcom::collective::barrier();
    }
    
    mgcom::finalize();
    
    return 0;
}

