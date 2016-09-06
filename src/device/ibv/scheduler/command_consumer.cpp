
#include "qp_buffer.hpp"
#include "command_consumer.hpp"
#include "send_wr_buffer.hpp"
#include <mgbase/thread.hpp>
#include <vector>
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"
#include "device/ibv/command/set_command_to.hpp"
#include <mgbase/ult/this_thread.hpp>
#include <mgbase/container/index_list.hpp>
#include "device/ibv/command/completion_selector.hpp"

namespace mgcom {
namespace ibv {

class command_consumer::impl
{
public:
    impl(command_queue& queue, const config& conf)
        : queue_(queue)
        , conf_(conf)
        , finished_{false}
        , qps_(conf.num_procs)
        , proc_indexes_(conf.num_procs)
    {
        for (process_id_t index = 0; index < conf.num_procs; ++index)
        {
            const process_id_t proc = conf.proc_first + index;
            
            qps_[index] = new qp_buffer(conf.ep, conf.alloc, proc);
            
            const auto qp_num = conf.ep.get_qp_num_of_proc(proc, 0);
            
            conf.comp_sel.set(qp_num, qps_[index]->get_completer());
        }
        
        #if 0
        for (process_id_t proc_index = 0; proc_index < conf.num_procs; ++proc_index)
        {
            const process_id_t proc = conf.proc_first + proc_index;
            
            for (index_t qp_index = 0; qp_index < conf.ep.get_qp_count(); ++qp_index)
            {
                const index_t index = proc_index * conf.ep.get_qp_count() + qp_index;
                
                qps_[index] = new qp_buffer(conf.ep, conf.alloc, proc, qp_index);
                
                const auto qp_num = conf.ep.get_qp_num_of_proc(proc, qp_index);
                
                conf.comp_sel.set(qp_num, qps_[index]->get_completer());
            }
        }
        #endif
        
        th_ = mgbase::thread(starter{*this});
    }
    
    virtual ~impl()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        // Join the running thread.
        th_.join();
        
        MGBASE_RANGE_BASED_FOR(auto&& qp, qps_) {
            delete qp;
        }
    }
    
private:
    struct starter
    {
        impl& self;
        
        void operator() () {
            self.loop();
        }
    };
    
    void loop()
    {
        while (MGBASE_LIKELY(!finished_))
        {
            dequeue_some();
            
            post_all();
        }
    }
    
    void dequeue_some()
    {
        const auto proc_first = conf_.proc_first;
        
        mgbase::size_t num_dequeued = 0;
        
        auto ret = queue_.try_dequeue(send_wr_buffer::max_size);
        
        if (!ret.valid())
            return;
        
        // TODO: exception safety
        
        MGBASE_RANGE_BASED_FOR(auto&& cmd, ret) 
        {
            MGBASE_ASSERT(proc_first <= cmd.proc);
            MGBASE_ASSERT(cmd.proc < proc_first + conf_.num_procs);
            
            const auto proc_index = cmd.proc - proc_first;
            
            // Convert a command to a WR.
            if (!qps_[proc_index]->try_enqueue(cmd))
                break;
            
            if (MGBASE_UNLIKELY(
                ! proc_indexes_.exists(proc_index))
            ) {
                proc_indexes_.push_back(proc_index);
            }
            
            ++num_dequeued;
        }
        
        ret.commit(num_dequeued);
        
        if (num_dequeued > 0) {
            MGBASE_LOG_DEBUG(
                "msg:Dequeued IBV requests.\t"
                "num_dequeued:{}"
            ,    num_dequeued
            );
        }
        else {
            MGBASE_LOG_VERBOSE(
                "msg:No IBV requests are dequeued."
            );
        }
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
    command_queue& queue_;
    const config conf_;
    
    bool finished_;
    mgbase::thread th_;
    
    std::vector<qp_buffer*> qps_;
    mgbase::index_list<process_id_t> proc_indexes_;
};

command_consumer::command_consumer(const config& conf)
    : impl_{mgbase::make_unique<impl>(static_cast<command_queue&>(*this), conf)} { }

command_consumer::~command_consumer() = default;

} // namespace ibv
} // namespace mgcom

