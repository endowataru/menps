
#include "unittest.hpp"

struct test_handler {
    static const mgcom::rpc::handler_id_t handler_id = 1;
    
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
    mgcom::rpc::register_handler2(mgcom::rpc::requester::get_instance(), test_handler{});
    
    int x = 0;
    int* dest = &x;
    mgcom::collective::broadcast(0, &dest, 1);
    
    int result = -1;
    
    {
        const auto rply_msg =
            mgcom::rpc::call<test_handler>(
                mgcom::rpc::requester::get_instance()
            ,   0
            ,   dest
            );
        
        result = *rply_msg;
    }
    
    mgcom::collective::barrier();
    
    if (mgcom::current_process_id() == 0) {
        ASSERT_EQ(static_cast<int>(mgcom::number_of_processes()) * 10, x);
    }
    
    ASSERT_EQ(mgcom::current_process_id(), static_cast<mgcom::process_id_t>(result));
}



