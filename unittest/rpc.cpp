
#include "unittest.hpp"

struct test_handler {
    static const mgcom::rpc::handler_id_t handler_id = 1;
    
    typedef int*    argument_type;
    typedef int     return_type;
    
    static return_type on_request(const mgcom::rpc::handler_parameters& params, const argument_type& arg) {
        *arg += 10;
        return static_cast<int>(params.source);
    }
};

TEST(Rpc, Roundtrip)
{
    mgcom::rpc::register_handler<test_handler>();
    
    int x = 0;
    int* dest = &x;
    mgcom::collective::broadcast(0, &dest, 1);
    
    int result = -1;
    
    mgcom::rpc::remote_call<test_handler>(
        0
    ,   dest
    ,   &result
    );
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        ASSERT_EQ(static_cast<int>(mgcom::number_of_processes()) * 10, x);
    }
    
    ASSERT_EQ(mgcom::current_process_id(), static_cast<mgcom::process_id_t>(result));
}



