
#pragma once

#include "device/ibv/native/completion_queue.hpp"
#include "completer.hpp"
#include <mgbase/thread.hpp>

namespace mgcom {
namespace ibv {

class poll_thread
{
public:
    poll_thread(completion_queue& cq, completer& comp)
        : cq_(&cq)
        , completer_(&comp) { }
    
    void start()
    {
        finished_ = false;
        
        th_ = mgbase::thread(
            mgbase::bind1st_of_1(
                MGBASE_MAKE_INLINED_FUNCTION(pass)
            ,   mgbase::wrap_reference(*this)
            )
        );
    }
    
    void stop()
    {
        finished_ = true;
        
        th_.join();
    }
    
    bool try_complete(
        const mgbase::operation& on_complete
    ,   mgbase::uint64_t* const wr_id_result
    ) {
        return completer_->try_complete(on_complete, wr_id_result);
    }
    
    void failed(const mgbase::uint64_t wr_id)
    {
        completer_->failed(wr_id);
    }
    
private:
    static void pass(poll_thread& self)
    {
        self.loop();
    }
    
    void loop()
    {
        MGBASE_LOG_DEBUG("msg:Started IBV polling.");
        
        while (MGBASE_LIKELY(!finished_))
        {
            ibv_wc wc;
            
            const int num = cq_->poll(&wc, 1);
            
            if (num == 1)
            {
                if (MGBASE_UNLIKELY(wc.status != IBV_WC_SUCCESS)) {
                    throw ibv_error("polling failed", wc.status);
                }
                
                completer_->notify(wc.wr_id);
                
                MGBASE_LOG_DEBUG(
                    "msg:Polled completion.\t"
                    "wr_id:{}"
                ,   wc.wr_id
                );
            }
            else
            {
                MGBASE_LOG_VERBOSE("msg:IBV CQ was empty.");
            }
        }
        
        MGBASE_LOG_DEBUG("msg:Finished IBV polling.");
    }
    
    /*int poll(
        ibv_wc* const   wc_array
    ,   const int       num_entries
    ) {
        const int ret = ibv_poll_cq(cq_, num_entries, wc_array);
        if (ret < 0)
            throw ibv_error();
        
        return ret;
    }*/
    
    bool finished_;
    completion_queue* cq_;
    completer* completer_;
    mgbase::thread th_;
};

} // namespace ibv
} // namespace mgcom

