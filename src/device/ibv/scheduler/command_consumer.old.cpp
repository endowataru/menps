#include "command_consumer.hpp"
#include "send_wr_buffer.hpp"
#include <mgbase/thread.hpp>
#include <vector>
#include "device/ibv/native/endpoint.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"
#include "device/ibv/command/set_command_to.hpp"
#include <mgbase/ult/this_thread.hpp>

namespace mgcom {
namespace ibv {

class command_consumer::impl
{
public:
    impl(command_queue& queue, const config& conf)
        : queue_(queue)
        , finished_{false}
        , wr_bufs_{new send_wr_buffer[conf.num_procs]}
        , sges_(conf.num_procs, std::vector<scatter_gather_entry>(send_wr_buffer::max_size))
        , conf_(conf)
    {
        th_ = mgbase::thread(starter{*this});
    }
    
    virtual ~impl()
    {
        // Order the running thread to stop.
        finished_ = true;
        
        queue_.notify();
        
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
        auto& queue = queue_;
        
        const auto proc_first = conf_.proc_first;
        const auto num_procs = conf_.num_procs;
        
        mgbase::scoped_ptr<mgbase::size_t []> num_wrs(new mgbase::size_t[num_procs]);
        
        for (mgbase::size_t i = 0; i < num_procs; ++i)
            num_wrs[i] = 0;
        
        while (MGBASE_LIKELY(!finished_))
        {
            auto ret = queue.dequeue(send_wr_buffer::max_size);
            
            MGBASE_RANGE_BASED_FOR(auto&& cmd, ret) 
            {
            /*#ifdef MGBASE_CXX11_RANGE_BASED_FOR_SUPPORTED
            for (auto&& cmd : ret) {
            #else
            // TODO: Replace with range-based for macro
            auto first = ret.begin();
            auto last = ret.end();
            
            for ( ; first != last; ++first)
            {
                auto&& cmd = *first;
            #endif*/
                
                MGBASE_ASSERT(cmd.proc >= proc_first);
                
                const auto index = cmd.proc - proc_first;
                
                auto& buf = wr_bufs_[index];
                
                auto& wr = buf.at(num_wrs[index]);
                auto& sge = sges_[index][num_wrs[index]];
                
                set_command_to(cmd, &wr, &sge); // FIXME
                
                ++num_wrs[index];
            }
            
            for (mgbase::size_t i = 0; i < num_procs; ++i)
            {
                const process_id_t proc = i + proc_first;
                
                const auto n = num_wrs[i];
                
                if (n > 0)
                {
                    num_wrs[i] = 0;
                    
                    auto& buf = wr_bufs_[i];
                    
                    auto link = buf.terminate_at(n - 1);
                    
                    while (MGBASE_UNLIKELY(
                        ! conf_.ep.try_post_send(proc, buf.at(0))
                    ))
                    {
                        mgbase::ult::this_thread::yield();
                    }
                    
                    buf.link_to(n - 1, link);
                }
            }
        }
    }
    
private:
    command_queue& queue_;
    bool finished_;
    mgbase::thread th_;
    mgbase::scoped_ptr<send_wr_buffer []> wr_bufs_;
    std::vector<std::vector<scatter_gather_entry>> sges_;
    const config conf_;
};

command_consumer::command_consumer(const config& conf)
    : impl_{mgbase::make_unique<impl>(static_cast<command_queue&>(*this), conf)} { }

command_consumer::~command_consumer() = default;

} // namespace ibv
} // namespace mgcom

