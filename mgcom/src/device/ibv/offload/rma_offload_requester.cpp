
#include "device/ibv/rma/requester.hpp"
#include "rma_executor.hpp"
#include "common/rma/offload_requester.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/shared_ptr.hpp>

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class rma_offload_requester
    : public rma::offload_requester
{
public:
    explicit rma_offload_requester(const requester_config& conf)
        : rma::offload_requester(
            rma::offload_requester::config{ 
                conf.ep.number_of_processes()
            ,   conf.num_qps_per_proc
            }
        )
        , conf_(conf)
        , execs_(
            mgbase::make_unique<mgbase::unique_ptr<mgbase::unique_ptr<rma_executor> []> []>(
                conf.ep.number_of_processes()
            )
        )
    {
        const auto num_procs = conf.ep.number_of_processes();
        const auto num_qps_per_proc = conf.num_qps_per_proc;
        
        for (process_id_t proc = 0; proc < num_procs; ++proc)
        {
            const process_id_t proc_from = proc;
            const mgbase::size_t num_procs_per_group = 1;
            
            execs_[proc] = mgbase::make_unique<mgbase::unique_ptr<rma_executor> []>(num_qps_per_proc);
            
            for (mgbase::size_t qp_index = 0; qp_index < num_qps_per_proc; ++qp_index)
            {
                execs_[proc][qp_index] =
                    mgbase::make_unique<rma_executor>(
                        rma_executor::config{
                            conf.qps
                        ,   qp_index
                        ,   conf.alloc
                        ,   proc_from
                        ,   num_procs_per_group
                        ,   conf.reply_be
                        }
                    );
                
                this->set_command_queue(proc, qp_index, &execs_[proc][qp_index]->get_command_queue());
            }
        }
    }
    
    const requester_config conf_;
    mgbase::unique_ptr<mgbase::unique_ptr<mgbase::unique_ptr<rma_executor> []> []> execs_;
};

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_rma_offload_requester(const requester_config& conf) {
    return mgbase::make_unique<rma_offload_requester>(conf);
}

} // namespace ibv

namespace rma {

MGBASE_THREAD_LOCAL mgbase::size_t* offload_requester::que_indexes_ = 0/*MGBASE_NULLPTR*/;
    // TODO: MGBASE_NULLPTR doesn't work well in initialization in old GCC

} // namespace rma

} // namespace mgcom

