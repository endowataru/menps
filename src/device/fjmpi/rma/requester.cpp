
#include "rma.hpp"
#include "device/mpi/rma/atomic.hpp"
#include "device/fjmpi/fjmpi.hpp"
#include "device/fjmpi/scheduler/command_producer.hpp"
#include "requester_base.hpp"

namespace mgcom {

namespace rma {
namespace /*unnamed*/ {

using namespace fjmpi;

const int flag_patterns[] = {
    FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 | FJMPI_RDMA_PATH0
,   FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1 | FJMPI_RDMA_PATH0
,   FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2 | FJMPI_RDMA_PATH0
,   FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3 | FJMPI_RDMA_PATH0
/*,   FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 | FJMPI_RDMA_PATH1
,   FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1 | FJMPI_RDMA_PATH1
,   FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2 | FJMPI_RDMA_PATH1
,   FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3 | FJMPI_RDMA_PATH1
,   FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 | FJMPI_RDMA_PATH2
,   FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1 | FJMPI_RDMA_PATH2
,   FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2 | FJMPI_RDMA_PATH2
,   FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3 | FJMPI_RDMA_PATH2
,   FJMPI_RDMA_LOCAL_NIC0 | FJMPI_RDMA_REMOTE_NIC0 | FJMPI_RDMA_PATH3
,   FJMPI_RDMA_LOCAL_NIC1 | FJMPI_RDMA_REMOTE_NIC1 | FJMPI_RDMA_PATH3
,   FJMPI_RDMA_LOCAL_NIC2 | FJMPI_RDMA_REMOTE_NIC2 | FJMPI_RDMA_PATH3
,   FJMPI_RDMA_LOCAL_NIC3 | FJMPI_RDMA_REMOTE_NIC3 | FJMPI_RDMA_PATH3*/
};

class fjmpi_requester
    : public fjmpi_requester_base<fjmpi_requester>
    , public mpi::emulated_atomic_requester
{
public:
    typedef fjmpi::command_code command_code_type;
    
    explicit fjmpi_requester(command_producer& cp, rpc::requester& req)
        : mpi::emulated_atomic_requester(req)
        , cp_(cp)
        { }
    
private:
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(const command_code code, Func&& func)
    {
        return cp_.template try_enqueue<Params>(code, std::forward<Func>(func));
    }
    
    int select_flags() MGBASE_NOEXCEPT
    {
        const auto num_flags = sizeof(flag_patterns) / sizeof(flag_patterns[0]);
        
        // TODO : Allowing concurrent writes to flag_index_.
        return flag_patterns[flag_index_ ++ % num_flags];
    }
    
    command_producer& cp_;
    int flag_index_;
    
    friend class fjmpi_requester_base<fjmpi_requester>;
};

} // unnamed namespace
} // namespace rma

namespace fjmpi {

mgbase::unique_ptr<rma::requester> make_rma_requester(command_producer& cp, rpc::requester& req)
{
    return mgbase::make_unique<rma::fjmpi_requester>(cp, req);
}

} // namespace fjmpi

} // namespace mgcom

