
#include <menps/meth/misc.hpp>
#include <menps/mecom.hpp>
#include <time.h>
#include <sys/time.h>

namespace menps {
namespace meth {

struct get_time_usec_handler
{
    static const mecom::rpc::handler_id_t handler_id = 1020; // TODO
    
    typedef int                 request_type; // TODO: dummy
    typedef mefdn::uint64_t    reply_type;
    
    template <typename ServerCtx>
    typename ServerCtx::return_type operator() (ServerCtx& sc) const
    {
        auto rply = sc.make_reply();
        
        #ifdef __APPLE__
        timeval t;
        gettimeofday(&t, NULL);
        const auto ret = t.tv_sec + t.tv_usec * 1e-9;
        #else
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        const auto ret = ts.tv_sec * 1000000000L + ts.tv_nsec;
        #endif
        
        *rply = ret;
        
        return rply;
    }
};

void initialize_misc()
{
    mecom::rpc::register_handler2(
        mecom::rpc::requester::get_instance()
    ,   get_time_usec_handler{}
    );
}

mefdn::uint64_t get_time_usec()
{
    return * mecom::rpc::call<get_time_usec_handler>(
        mecom::rpc::requester::get_instance()
    ,   0 // TODO
    ,   0
    );
}

} // namespace meth
} // namespace menps

