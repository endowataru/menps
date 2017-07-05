
#include <mgth/misc.hpp>
#include <mgcom.hpp>
#include <time.h>

namespace mgth {

struct get_time_usec_handler
{
    static const mgcom::rpc::handler_id_t handler_id = 1020; // TODO
    
    typedef int                 request_type; // TODO: dummy
    typedef mgbase::uint64_t    reply_type;
    
    template <typename ServerCtx>
    typename ServerCtx::return_type operator() (ServerCtx& sc) const
    {
        auto rply = sc.make_reply();
        
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        *rply = ts.tv_sec * 1000000000L + ts.tv_nsec;
        
        return rply;
    }
};

void initialize_misc()
{
    mgcom::rpc::register_handler2(
        mgcom::rpc::requester::get_instance()
    ,   get_time_usec_handler{}
    );
}

mgbase::uint64_t get_time_usec()
{
    return * mgcom::rpc::call<get_time_usec_handler>(
        mgcom::rpc::requester::get_instance()
    ,   0 // TODO
    ,   0
    );
}

} // namespace mgth
