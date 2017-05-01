
#include "rma_executor.hpp"
#include "set_command_to_2.hpp"
#include "device/ibv/scheduler/qp_buffer.hpp"
#include "device/ibv/scheduler/send_wr_buffer.hpp"
#include "common/basic_offload_thread.hpp"
#include <mgcom/ult.hpp>
#include <mgbase/container/index_list.hpp>
#include <mgbase/vector.hpp>
#include <mgbase/shared_ptr.hpp>

namespace mgcom {
namespace ibv {

struct rma_executor_policy
{
    typedef rma_executor::impl  derived_type;
    typedef ult::thread         thread_type;
};

class rma_executor::impl
    : public basic_offload_thread<rma_executor_policy>
    , public rma::command_queue
{
    typedef basic_offload_thread<rma_executor_policy>   base;
    typedef rma::command_queue                          queue_type;
    
public:
    explicit impl(const config& conf)
        : conf_(conf)
        , qps_(conf.num_procs)
        , proc_indexes_(conf.num_procs)
    {
        for (process_id_t index = 0; index < conf.num_procs; ++index)
        {
            const process_id_t proc = conf.proc_first + index;
            
            auto& qp = conf_.qps.get_qp(proc, conf_.qp_index);
            auto& comp_sel = conf_.qps.get_comp_sel(proc, conf_.qp_index);
            auto& tag_que = conf_.qps.get_tag_queue(proc, conf_.qp_index);
            
            qps_[index] = mgbase::make_shared<qp_buffer>(
                qp_buffer::config{ qp, tag_que, conf_.alloc, comp_sel, conf_.reply_be }
            );
        }
        
        this->start();
    }
    
private:
    friend class basic_offload_thread<rma_executor_policy>;
    
    queue_type::dequeue_transaction try_dequeue()
    {
        return queue_type::try_dequeue(send_wr_buffer::max_size);
    }
    
    template <typename Command>
    bool try_execute(Command&& cmd)
    {
        const auto proc = cmd.arg.proc;
        
        MGBASE_ASSERT(conf_.proc_first <= proc);
        MGBASE_ASSERT(proc < conf_.proc_first + conf_.num_procs);
        
        const auto proc_index = proc - conf_.proc_first;
        
        // Convert a command to a WR.
        if (!qps_[proc_index]->try_enqueue(cmd)) {
            return false;
        }
        
        if (MGBASE_UNLIKELY(
            ! proc_indexes_.exists(proc_index)
        )) {
            proc_indexes_.push_back(proc_index);
        }
        
        return true;
    }
    
    bool has_remaining()
    {
        return ! proc_indexes_.empty();
    }
    
    void post_all()
    {
        const auto proc_first = conf_.proc_first;
        
        for (auto itr = proc_indexes_.begin(); itr != proc_indexes_.end(); )
        {
            const auto proc_index = *itr;
            
            const auto proc = proc_first + proc_index;
            MGBASE_ASSERT(proc < proc_first + conf_.num_procs);
            
            if (qps_[proc_index]->try_post_all()) {
                // Erase the corresponding process ID from process index list.
                itr = proc_indexes_.erase(itr);
            }
            else {
                // Increment the iterator
                // only when there are at least one WR for that process (ID).
                ++itr;
            }
        }
    }
    
private:
    const config conf_;
    
    mgbase::vector<mgbase::shared_ptr<qp_buffer>> qps_;
    mgbase::index_list<process_id_t> proc_indexes_;
};

rma_executor::rma_executor(const config& conf)
    : impl_{mgbase::make_unique<impl>(conf)} { }

rma_executor::~rma_executor() = default;

rma::command_queue& rma_executor::get_command_queue() {
    return *impl_;
}

} // namespace ibv
} // namespace mgcom
