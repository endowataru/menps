
#include "command_consumer.hpp"
#include "send_wr_buffer.hpp"
#include <mgbase/thread.hpp>
#include <vector>
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"
#include "device/ibv/command/set_command_to.hpp"
#include <mgbase/ult/this_thread.hpp>
#include <mgbase/container/index_list.hpp>

namespace mgcom {
namespace ibv {

class command_consumer::impl
{
public:
    impl(command_queue& queue, const config& conf)
        : queue_(queue)
        , conf_(conf)
        , finished_{false}
        , wr_bufs_{new send_wr_buffer[conf.num_procs]}
        , sges_(conf.num_procs, std::vector<scatter_gather_entry>(send_wr_buffer::max_size))
        , proc_indexes_(conf.num_procs)
    {
        th_ = mgbase::thread(starter{*this});
    }
    
    virtual ~impl()
    {
        // Order the running thread to stop.
        finished_ = true;
        
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
            if (!proc_indexes_.exists(proc_index)) {
                proc_indexes_.push_back(proc_index);
            }
            
            auto& buf = wr_bufs_[proc_index];
            
            mgbase::size_t wr_index = 0;
            const auto wr = buf.try_enqueue(&wr_index);
            if (!wr) break;
            
            auto& sge = sges_[proc_index][wr_index];
            
            set_command_to(cmd, wr, &sge); // FIXME
            
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
            
            auto& buf = wr_bufs_[proc_index];
            
            MGBASE_ASSERT(!buf.empty());
            
            buf.terminate();
            
            ibv_send_wr* bad_wr = MGBASE_NULLPTR;
            
            while (MGBASE_UNLIKELY(
                ! conf_.ep.try_post_send(proc, buf.front(), &bad_wr)
            ))
            {
                mgbase::ult::this_thread::yield();
            }
            
            buf.relink();
            
            buf.consume(bad_wr);
            
            if (bad_wr == MGBASE_NULLPTR) {
                // Erase the corresponding process ID from process index list.
                itr = proc_indexes_.erase(itr);
            }
            else {
                // Increment the iterator
                // only when there are at least one WR for that process (ID).
                ++itr;
            }
            
            MGBASE_LOG_DEBUG(
                "msg:Posted IBV requests.\t"
                "proc:{}\tbad_wr:{:x}"
            ,   proc
            ,   reinterpret_cast<mgbase::uintptr_t>(bad_wr)
            );
        }
    }
    
private:
    command_queue& queue_;
    const config conf_;
    
    bool finished_;
    mgbase::thread th_;
    
    mgbase::scoped_ptr<send_wr_buffer []>           wr_bufs_;
    std::vector<std::vector<scatter_gather_entry>>  sges_;
    mgbase::index_list<process_id_t>                proc_indexes_;
};

command_consumer::command_consumer(const config& conf)
    : impl_{mgbase::make_unique<impl>(static_cast<command_queue&>(*this), conf)} { }

command_consumer::~command_consumer() = default;

} // namespace ibv
} // namespace mgcom

