
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

struct am_test {
    static const mgcom::am::handler_id_t request_id = 0;
    static const mgcom::am::handler_id_t reply_id   = 1;
    
    struct argument_type { };
    typedef void return_type;
    
    static return_type on_request(const mgcom::am::callback_parameters& /*params*/, const argument_type& /*arg*/)
    {
        return;
    }
};

int main(int argc, char* argv[])
{
    mgcom::initialize(&argc, &argv);
    
    const mgcom::process_id_t root_proc = 0;
    const int number_of_trials = atoi(argv[1]);
    
    mgcom::am::register_roundtrip_handler<am_test>();
    
    mgcom::collective::barrier();
    
    double clocks = 0;
    
    if (mgcom::current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mgbase::stopwatch sw;
            sw.start();
            
            mgcom::am::call_roundtrip_cb cb;
            mgcom::am::call_roundtrip_nb<am_test>(cb, root_proc, am_test::argument_type()).wait();
            
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

