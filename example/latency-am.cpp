
#include <mgcom.hpp>
#include <mpi.h>

namespace {

void f(const mgcom::am::callback_parameters* params) {
    int val = *static_cast<const int*>(params->data);
    std::cout << "f: " << val << " @ " << mgcom::current_process_id() << std::endl;
    
    int result = val + 1;
    
    mgcom::am::reply(params, &result, sizeof(int));
}

}

int main(int argc, char* argv[])
{
    using namespace mgcom;
    
    initialize(&argc, &argv);
    
    am::register_handler(0, f);
    
    if (current_process_id() == 0) {
        am::send_cb cb;
        int val = 123;
        am::send(&cb, 0, &val, sizeof(val), 1);
        while (mgbase::async_test(cb)) { }
    }
    
    barrier();
    
    finalize();
    
    return 0;
}

