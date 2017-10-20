
#include "unittest.hpp"

struct test_handler {
    static const mecom::rpc::handler_id_t handler_id = 1;
    
    typedef int*    request_type;
    typedef int     reply_type;
    
    template <typename ServerCtx>
    typename ServerCtx::return_type operator() (ServerCtx& sc) const
    {
        const auto src_proc = sc.src_proc();
        const auto& rqst = sc.request();
        *rqst += 10;
        
        auto rply = sc.make_reply();
        *rply = static_cast<int>(src_proc);
        return rply;
    }
};

TEST(Rpc, Roundtrip)
{
    mecom::rpc::register_handler2(mecom::rpc::requester::get_instance(), test_handler{});
    
    int x = 0;
    int* dest = &x;
    mecom::collective::broadcast(0, &dest, 1);
    
    int result = -1;
    
    {
        const auto rply_msg =
            mecom::rpc::call<test_handler>(
                mecom::rpc::requester::get_instance()
            ,   0
            ,   dest
            );
        
        result = *rply_msg;
    }
    
    mecom::collective::barrier();
    
    if (mecom::current_process_id() == 0) {
        ASSERT_EQ(static_cast<int>(mecom::number_of_processes()) * 10, x);
    }
    
    ASSERT_EQ(mecom::current_process_id(), static_cast<mecom::process_id_t>(result));
}



