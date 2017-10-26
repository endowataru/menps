
#include "qp_buffer.hpp"
#include "command_consumer.hpp"
#include "send_wr_buffer.hpp"
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/command/set_command_to.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include <menps/mecom/ult.hpp>
#include <menps/mefdn/container/index_list.hpp>
#include <menps/mefdn/vector.hpp>
#include <menps/mefdn/memory/shared_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class command_consumer::impl
{
public:
    explicit impl(command_queue& queue, const config& conf)
        : queue_(queue)
        , conf_(conf)
        , finished_{false}
        , qps_(conf.num_procs)
        , proc_indexes_(conf.num_procs)
    {
        for (process_id_t index = 0; index < conf.num_procs; ++index)
        {
            const process_id_t proc = conf.proc_first + index;
            
            auto& qp = conf_.qps.get_qp(proc, conf_.qp_index);
            
            auto& comp_sel = conf_.qps.get_comp_sel(proc, conf_.qp_index);
            auto& tag_que = conf_.qps.get_tag_queue(proc, conf_.qp_index);
            
            qps_[index] = mefdn::make_shared<qp_buffer>(
                qp_buffer::config{ qp, tag_que, conf_.alloc, comp_sel, conf_.reply_be }
            );
        }
        
        th_ = ult::thread(starter{*this});
    }
    
    virtual ~impl()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        #ifdef MECOM_IBV_ENABLE_SLEEP_QP
        this->queue_.notify();
        #endif
        
        // Join the running thread.
        th_.join();
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
        while (MEFDN_LIKELY(!finished_))
        {
            dequeue_some();
            
            post_all();
        }
    }
    
    void dequeue_some()
    {
        const auto proc_first = conf_.proc_first;
        
        mefdn::size_t num_dequeued = 0;
        
        auto ret = queue_.try_dequeue(send_wr_buffer::max_size);
        
        if (MEFDN_UNLIKELY(!ret.valid())) {
            #ifdef MECOM_IBV_ENABLE_SLEEP_QP
            auto lk = this->queue_.get_lock();
            
            if (!proc_indexes_.empty()) {
                lk.unlock();
                
                ult::this_thread::yield();
                return;
            }
            
            if (queue_.try_sleep()) {
                MEFDN_LOG_DEBUG(
                    "msg:Command consumer starts sleeping."
                );
                
                queue_.wait(lk);
            } else {
                // Finish the critical section because this thread failed to sleep.
                lk.unlock();
                
                ult::this_thread::yield();
            }
            #endif
            
            return;
        }
        
        // TODO: exception safety
        
        MEFDN_RANGE_BASED_FOR(auto&& cmd, ret) 
        {
            MEFDN_ASSERT(proc_first <= cmd.proc);
            MEFDN_ASSERT(cmd.proc < proc_first + conf_.num_procs);
            
            const auto proc_index = cmd.proc - proc_first;
            
            // Convert a command to a WR.
            if (!qps_[proc_index]->try_enqueue(cmd))
                break;
            
            if (MEFDN_UNLIKELY(
                ! proc_indexes_.exists(proc_index))
            ) {
                proc_indexes_.push_back(proc_index);
            }
            
            ++num_dequeued;
        }
        
        ret.commit(num_dequeued);
        
        if (num_dequeued > 0) {
            MEFDN_LOG_DEBUG(
                "msg:Dequeued IBV requests.\t"
                "num_dequeued:{}"
            ,    num_dequeued
            );
        }
        else {
            MEFDN_LOG_VERBOSE(
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
            MEFDN_ASSERT(proc < proc_first + conf_.num_procs);
            
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
    ult::thread th_;
    
    mefdn::vector<mefdn::shared_ptr<qp_buffer>> qps_;
    mefdn::index_list<process_id_t> proc_indexes_;
};

command_consumer::command_consumer(const config& conf)
    : impl_{mefdn::make_unique<impl>(static_cast<command_queue&>(*this), conf)} { }

command_consumer::~command_consumer() = default;

} // namespace ibv
} // namespace mecom
} // namespace menps

