
#include "device/ibv/rma/requester.hpp"
#include "device/ibv/rma/requester_base.hpp"
#include "serializer.hpp"
#include <mgbase/arithmetic.hpp>
#include <mgbase/algorithm.hpp>
#include <mgbase/logger.hpp>
#include <mgbase/shared_ptr.hpp>
#include "device/ibv/command/poll_thread.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include "device/ibv/native/alltoall_queue_pairs.hpp"

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
        #if 0
        num_grouped_procs_ = get_num_grouped_procs();
        #endif
        const auto num_grouped_procs = get_num_grouped_procs();
        num_groups_ = mgbase::roundup_divide(num_procs, num_grouped_procs);
        
        sers_.resize(num_groups_);
        #ifdef MGCOM_IBV_SEPARATE_CQ
        comp_sels_.resize(num_groups_);
        poll_ths_.resize(num_groups_);
        #endif
        
        const auto num_qps_per_proc = conf_.num_qps_per_proc;
        
        mgbase::size_t proc_from = 0;
        
        for (mgbase::size_t group_index = 0; group_index < num_groups_; ++group_index)
        {
            sers_[group_index].resize(num_qps_per_proc);
            
            #ifdef MGCOM_IBV_SEPARATE_CQ
            comp_sels_[group_index].resize(num_qps_per_proc);
            poll_ths_[group_index].resize(num_qps_per_proc);
            #endif
            
            for (mgbase::size_t qp_index = 0; qp_index < num_qps_per_proc; ++qp_index)
            {
                mgbase::size_t num_procs_per_group = mgbase::min(proc_from + num_grouped_procs, num_procs) - proc_from;
                
                MGBASE_LOG_DEBUG(
                    "msg:Initializing serializer.\t"
                    "from:{}\tnum:{}"
                ,   proc_from
                ,   num_procs_per_group
);
                
                #ifdef MGCOM_IBV_SEPARATE_CQ
                comp_sels_[group_index][qp_index] =
                    mgbase::make_shared<completion_selector>();
                
                // FIXME: broken when num_groups != num_procs
                auto& cq = conf_.qps.get_cq(group_index, qp_index);
                
                poll_ths_[group_index][qp_index] =
                    mgbase::make_shared<poll_thread>(
                        cq
                    ,   *comp_sels_[group_index][qp_index]
                    );
                #endif
                
                sers_[group_index][qp_index] =
                    mgbase::make_shared<serializer>(
                        serializer::config{
                            conf_.qps
                        ,   qp_index
                        ,   conf_.alloc
                        #ifdef MGCOM_IBV_SEPARATE_CQ
                        ,   *comp_sels_[group_index][qp_index]
                        #else
                        ,   conf_.comp_sel
                        #endif
                        ,   proc_from
                        ,   num_procs_per_group
                        ,   conf.reply_be
                        }
                    );
            }
            
            proc_from += num_grouped_procs;
            
            if (proc_from >= num_procs)
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
        
        const mgbase::size_t group_index = proc / this->get_num_grouped_procs();
        MGBASE_ASSERT(group_index < sers_.size());
        
        auto qp_indexes = qp_indexes_;
        if (MGBASE_UNLIKELY(qp_indexes == MGBASE_NULLPTR)) {
            qp_indexes_ = qp_indexes = new mgbase::size_t[num_groups_]; // value-initialization
            // TODO : memory leak
            
            std::uninitialized_fill_n(qp_indexes, num_groups_, 0);
        }
        
        const auto qp_index = qp_indexes[group_index];
        MGBASE_ASSERT(qp_index < conf_.num_qps_per_proc);
        
        if (++qp_indexes[group_index] >= conf_.num_qps_per_proc) {
            qp_indexes[group_index] = 0;
        }
        
        return sers_[group_index][qp_index]
            ->try_enqueue<Params>(proc, code, std::forward<Func>(func));
    }
    
    static mgbase::size_t get_num_grouped_procs() MGBASE_NOEXCEPT
    {
    #if 0
        if (const auto str = std::getenv("MGCOM_IBV_NUM_GROUP_PROCS"))
            return std::atoi(str);
        else
    #endif
            return 1; // Default
    }
    
    const requester_config conf_;
    #if 0
    mgbase::size_t num_grouped_procs_;
    #endif
    mgbase::size_t num_groups_;
    std::vector<std::vector<mgbase::shared_ptr<serializer>>> sers_;
    
    static MGBASE_THREAD_LOCAL mgbase::size_t* qp_indexes_;
    
    #ifdef MGCOM_IBV_SEPARATE_CQ
    std::vector<std::vector<mgbase::shared_ptr<completion_selector>>> comp_sels_;
    std::vector<std::vector<mgbase::shared_ptr<poll_thread>>> poll_ths_;
    #endif
};

MGBASE_THREAD_LOCAL mgbase::size_t* scheduled_rma_requester::qp_indexes_ = 0/*MGBASE_NULLPTR*/;
    // TODO: MGBASE_NULLPTR doesn't work well in initialization in old GCC

} // unnamed namespace

mgbase::unique_ptr<rma::requester> make_scheduled_rma_requester(const requester_config& conf)
{
    return mgbase::make_unique<scheduled_rma_requester>(conf);
}

} // namespace ibv
} // namespace mgcom

