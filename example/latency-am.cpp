
#include <mgcom.hpp>
#include <mpi.h>

namespace {

void f(const mgcom::am_callback_parameter* param) {
    int val = *static_cast<const int*>(param->data);
    std::cout << "f: " << val << " @ " << mgcom::current_process_id() << std::endl;
}

}

int main(int argc, char* argv[])
{
    using namespace mgcom;
    
    initialize(&argc, &argv);
    
    register_am_handler(0, f);
    
    if (current_process_id() == 0) {
        send_am_cb cb;
        int val = 123;
        send_am(&cb, 0, &val, sizeof(val), 1);
        while (mgbase::async_test(cb)) { }
    }
    
    barrier();
    
    finalize();
    
    return 0;
}

