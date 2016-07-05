
#include "poll_thread.hpp"
#include "completer.hpp"
#include <mgbase/thread.hpp>
#include <mgbase/logger.hpp>
#include "device/ibv/native/completion_queue.hpp"

namespace mgcom {
namespace ibv {

class poll_thread::impl
{
public:
    impl(completion_queue& cq, completer& comp)
        : finished_{false}
        , cq_(cq)
        , completer_(comp)
    {
        th_ = mgbase::thread(
            mgbase::bind1st_of_1(
                MGBASE_MAKE_INLINED_FUNCTION(pass)
            ,   mgbase::wrap_reference(*this)
            )
        );
    }
    
    ~impl()
    {
        finished_ = true;
        
        th_.join();
    }
    
private:
    static void pass(impl& self)
    {
        self.loop();
    }
    
    void loop()
    {
        MGBASE_LOG_DEBUG("msg:Started IBV polling.");
        
        while (MGBASE_LIKELY(!finished_))
        {
            ibv_wc wc;
            
            const int num = cq_.poll(&wc, 1);
            
            if (num == 1)
            {
                if (MGBASE_UNLIKELY(wc.status != IBV_WC_SUCCESS)) {
                    throw ibv_error("polling failed", wc.status);
                }
                
                completer_.notify(wc.wr_id);
                
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
    
    bool finished_;
    completion_queue& cq_;
    completer& completer_;
    mgbase::thread th_;
};

poll_thread::poll_thread(completion_queue& cq, completer& comp)
    : impl_{ mgbase::make_unique<impl>(cq, comp) }
{ }

poll_thread::~poll_thread() = default;

} // namespace ibv
} // namespace mgcom

