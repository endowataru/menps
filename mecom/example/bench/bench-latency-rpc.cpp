
#include <menps/mecom.hpp>
#include <menps/mefdn/profiling/stopwatch.hpp>
#include <iostream>

namespace mefdn = menps::mefdn;
namespace mecom = menps::mecom;

struct bench_handler {
    static const mecom::rpc::handler_id_t handler_id = 1;
    
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
    mecom::initialize(&argc, &argv);
    
    const mecom::process_id_t root_proc = 0;
    const int number_of_trials = atoi(argv[1]);
    
    mecom::rpc::register_handler2(mecom::rpc::requester::get_instance(), bench_handler{});
    
    mecom::collective::barrier();
    
    double clocks = 0;
    
    if (mecom::current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mefdn::stopwatch sw;
            sw.start();
            
            mecom::rpc::call<bench_handler>(
                mecom::rpc::requester::get_instance()
            ,   root_proc
            ,   bench_handler::request_type()
            );
            
            clocks += sw.elapsed();
        }
    }
    
    mecom::collective::barrier();
    
    for (mecom::process_id_t proc = 0; proc < mecom::number_of_processes(); ++proc) {
        if (proc == mecom::current_process_id())
            std::cout << mecom::current_process_id() << "\t" << (clocks / number_of_trials) << std::endl;
        
        mecom::collective::barrier();
    }
    
    mecom::finalize();
    
    return 0;
}

