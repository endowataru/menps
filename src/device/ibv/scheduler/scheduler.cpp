
#include "device/ibv/rma/requester.hpp"
#include "device/ibv/rma/requester_base.hpp"
#include "serializer.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/algorithm.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/shared_ptr.hpp>

namespace mgcom {
namespace ibv {

namespace /*unnamed*/ {

class scheduled_rma_requester
    : public rma::ibv_requester_base<scheduled_rma_requester>
{
public:
    typedef ibv::command_code   command_code_type;
    
    scheduled_rma_requester(const requester_config& conf)
        : conf_(conf)
    {
        const auto num_procs = conf_.ep.number_of_processes();
        
        const mgbase::size_t max_num_offload_threads = get_max_num_offload_threads();
        
        sers_.resize(max_num_offload_threads);
        
        qp_per_thread_ = mgbase::roundup_divide(num_procs, max_num_offload_threads);
        
        mgbase::size_t qp_from = 0;
        
        for (mgbase::size_t index = 0; index < max_num_offload_threads; ++index)
        {
            mgbase::size_t num_qps = mgbase::min(qp_from + qp_per_thread_, num_procs) - qp_from;
            
            MGBASE_LOG_DEBUG(
                "msg:Initializing serializer.\t"
                "from:{}\tnum:{}"
            ,   qp_from
            ,   num_qps
            );
            
            sers_[index] = mgbase::make_shared<serializer>(
                serializer::config{ conf_.qps, conf_.alloc, conf_.comp_sel, qp_from, num_qps, conf.reply_be }
            );
            
            qp_from += qp_per_thread_;
            
            if (qp_from >= num_procs)
                break;
        }
        /*
        for (process_id_t proc = 0; proc < ep.number_of_processes(); ++proc)
        {
            sers_[proc] = new serializer(
                serializer::config{ ibv_ep, alloc, comp_sel, proc, 1 }
            );
        }*/
    }
    
    scheduled_rma_requester(const scheduled_rma_requester&) = delete;
    scheduled_rma_requester& operator = (const scheduled_rma_requester&) = delete;
    
private:
    friend class rma::ibv_requester_base<scheduled_rma_requester>;
    
    template <typename Params, typename Func>
    MGBASE_WARN_UNUSED_RESULT
    bool try_enqueue(
        const process_id_t      proc
    ,   const command_code_type code
    ,   Func&&                  func
    ) {
        MGBASE_ASSERT(valid_process_id(proc));
        
        const mgbase::size_t i = proc / qp_per_thread_;
        
        MGBASE_ASSERT(i < sers_.size());
        
        return sers_[i]->try_enqueue<Params>(proc, code, std::forward<Func>(func));
    }
    
    static mgbase::size_t get_max_num_offload_threads() MGBASE_NOEXCEPT
    {
        if (const char* const direct = std::getenv("MGCOM_IBV_MAX_NUM_OFFLOAD_THREADS"))
            return std::atoi(direct);
        else
            return 1; // Default
    }
    
    const requester_config conf_;
    mgbase::size_t qp_per_thread_;
    std::vector<mgbase::shared_ptr<serializer>> sers_;
};

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(const requester_config& conf)
{
    return mgbase::make_unique<scheduled_rma_requester>(conf);
}

} // namespace ibv
} // namespace mgcom

