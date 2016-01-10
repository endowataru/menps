
#include "unittest.hpp"

struct test_handler {
    static const mgcom::am::handler_id_t request_id = 1;
    static const mgcom::am::handler_id_t reply_id   = 2;
    
    typedef int*    request_argument;
    typedef int     reply_argument;
    
    static reply_argument on_request(const mgcom::am::callback_parameters& params, const request_argument& arg) {
        *arg += 10;
        return static_cast<int>(params.source);
    }
};

TEST(Am, Roundtrip)
{
    mgcom::am::register_roundtrip_handler<test_handler>();
    
    int x = 0;
    int* dest = &x;
    mgcom::collective::broadcast(0, &dest, 1);
    
    int result = -1;
    
    mgcom::am::call_roundtrip_cb cb;
    mgcom::am::call_roundtrip_nb<test_handler>(
        cb
    ,   0
    ,   dest
    ,   &result
    )
    .wait();
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        ASSERT_EQ(static_cast<int>(mgcom::number_of_processes()) * 10, x);
    }
    
    ASSERT_EQ(mgcom::current_process_id(), static_cast<mgcom::process_id_t>(result));
}

