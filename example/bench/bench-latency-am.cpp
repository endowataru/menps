
#include <mgcom.hpp>
#include <mgbase/profiling/stopwatch.hpp>
#include <iostream>

namespace {

struct argument {
    bool* ptr;
    //int   value;
};

struct constants {
    static const mgcom::am::handler_id_t id_f = 0;
    static const mgcom::am::handler_id_t id_g = 1;
};

void g(const mgcom::am::callback_parameters* params) {
    const argument& arg = *static_cast<const argument*>(params->data);
    
    //std::cout << "g: " << arg.value << " @ " << mgcom::current_process_id() << std::endl;
    
    *arg.ptr = true;
}

void f(const mgcom::am::callback_parameters* params) {
    const argument& arg = *static_cast<const argument*>(params->data);
    
    //std::cout << "f: " << arg.value << " @ " << mgcom::current_process_id() << std::endl;
    
    argument sent_arg;
    sent_arg.ptr = arg.ptr;
    //sent_arg.value = arg.value + 100;
    
    mgcom::am::reply(params, constants::id_g, &sent_arg, sizeof(argument));
}

}

int main(int argc, char* argv[])
{
    using namespace mgcom;
    
    const process_id_t root_proc = 0;
    const int number_of_trials = atoi(argv[1]);
    
    initialize(&argc, &argv);
    
    am::register_handler(constants::id_f, f);
    am::register_handler(constants::id_g, g);
    
    barrier();
    
    double clocks = 0;
    
    if (current_process_id() != root_proc)
    {
        for (int i = 0; i < number_of_trials; i++) {
            mgbase::stopwatch sw;
            sw.start();
            
            am::send_cb cb;
            
            bool flag = false;
            
            argument arg;
            arg.ptr = &flag;
            //arg.value = 123;
            
            //std::cout << "A" << std::endl;
            am::send_nb(cb, constants::id_f, &arg, sizeof(argument), root_proc);
            //std::cout << "B" << std::endl;
            mgbase::control::wait(cb);
            //std::cout << "C" << std::endl;
            
            while (!flag)
                am::poll();
            
            clocks += sw.elapsed();
        }
    }
    
    barrier();
    
    std::cout << current_process_id() << " " << (clocks / number_of_trials) << std::endl;
    
    finalize();
    
    return 0;
}

