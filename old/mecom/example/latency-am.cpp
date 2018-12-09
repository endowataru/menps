
#include <menps/mecom.hpp>
#include <mpi.h>

namespace {

struct argument {
    bool* ptr;
    int   value;
};

void g(const mecom::am::callback_parameters* params) {
    const argument& arg = *static_cast<const argument*>(params->data);
    
    std::cout << "g: " << arg.value << " @ " << mecom::current_process_id() << std::endl;
    
    *arg.ptr = true;
}

void f(const mecom::am::callback_parameters* params) {
    const argument& arg = *static_cast<const argument*>(params->data);
    
    std::cout << "f: " << arg.value << " @ " << mecom::current_process_id() << std::endl;
    
    argument sent_arg;
    sent_arg.ptr = arg.ptr;
    sent_arg.value = arg.value + 100;
    
    mecom::am::reply(params, 1, &sent_arg, sizeof(argument));
}

}

int main(int argc, char* argv[])
{
    using namespace mecom;
    
    initialize(&argc, &argv);
    
    am::register_handler(0, f);
    am::register_handler(1, g);
    
    if (current_process_id() == 0) {
        am::send_cb cb;
        
        bool flag = false;
        
        argument arg;
        arg.ptr = &flag;
        arg.value = 123;
        
        std::cout << "A" << std::endl;
        am::send_nb(cb, 0, &arg, sizeof(argument), 1);
        std::cout << "B" << std::endl;
        mefdn::control::wait(cb);
        std::cout << "C" << std::endl;
        
        while (!flag)
            am::poll();
        
    }
    
    barrier();
    
    finalize();
    
    return 0;
}

