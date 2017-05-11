
#include "poll_thread.hpp"
#include <mgbase/logger.hpp>
#include "completion_selector.hpp"
#include <mgdev/ibv/ibv_error.hpp>
#include <mgcom/ult.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace ibv {

using mgdev::ibv::completion_queue;
using mgdev::ibv::ibv_error;

class poll_thread::impl
{
    static const mgbase::size_t max_num_polled = 1 << 12;
    
public:
    impl(completion_queue& cq, completion_selector& comp_sel)
        : finished_{false}
        , cq_(cq)
        , comp_sel_(comp_sel)
        , wcs_(mgbase::make_unique<ibv_wc []>(max_num_polled))
    {
        #if defined(MGCOM_IBV_ENABLE_SLEEP_CQ) && defined(MGCOM_FORK_COMPLETER_THREAD)
        comp_sel_.set_entrypoint(&impl::loop_start, this);
        #else
        th_ = ult::thread(starter{*this});
        #endif
    }
    
    ~impl()
    {
        finished_ = true;
        
        #ifdef MGCOM_IBV_ENABLE_SLEEP_CQ
        comp_sel_.force_notify();
        #endif
        
        #ifndef MGCOM_FORK_COMPLETER_THREAD
        th_.join();
        #endif
    }
    
private:
    struct starter
    {
        impl&   self;
        
        void operator() () const {
            self.loop();
        }
    };
    
    static void* loop_start(void* const self_ptr)
    {
        auto& self = *static_cast<impl*>(self_ptr);
        self.loop();
        
        return MGBASE_NULLPTR;
    }
    
    void loop()
    {
        /*#ifdef MGCOM_FORK_COMPLETER_THREAD
        ult::this_thread::yield();
        #endif*/
        
        MGBASE_LOG_DEBUG("msg:Started IBV polling.");
        
        while (MGBASE_LIKELY(!finished_))
        {
            const int ret = cq_.poll(wcs_.get(), max_num_polled);
            
            if (ret > 0)
            {
                const auto num = static_cast<mgbase::size_t>(ret);
                
                for (mgbase::size_t i = 0; i < num; ++i)
                {
                    auto& wc = wcs_[i];
                    
                    if (MGBASE_UNLIKELY(wc.status != IBV_WC_SUCCESS)) {
                        throw ibv_error("polling failed", wc.status);
                    }
                    
                    auto& comp = comp_sel_.get(wc.qp_num);
                    
                    comp.notify(wc.wr_id);
                    
                    MGBASE_LOG_DEBUG(
                        "msg:Polled completion.\t"
                        "wr_id:{}"
                    ,   wc.wr_id
                    );
                }
                
                #ifdef MGCOM_IBV_ENABLE_SLEEP_CQ
                #ifdef MGCOM_FORK_COMPLETER_THREAD
                if (comp_sel_.remove_outstanding_and_try_sleep(num)) {
                    ult::this_thread::detach();
                    return;
                }
                #else
                comp_sel_.remove_outstanding(num);
                #endif
                #endif
            }
            else
            {
                MGBASE_LOG_VERBOSE("msg:IBV CQ was empty.");
                
                #ifdef MGCOM_IBV_ENABLE_SLEEP_CQ
                #ifdef MGCOM_FORK_COMPLETER_THREAD
                ult::this_thread::yield();
                #else
                if (! comp_sel_.try_wait()) {
                    // While there are ongoing requests,
                    // polling is failing.
                    ult::this_thread::yield();
                }
                #endif
                #endif
            }
        }
        
        MGBASE_LOG_DEBUG("msg:Finished IBV polling.");
    }
    
    bool finished_;
    completion_queue& cq_;
    completion_selector& comp_sel_;
    ult::thread th_;
    mgbase::unique_ptr<ibv_wc []> wcs_;
};

poll_thread::poll_thread(completion_queue& cq, completion_selector& comp_sel)
    : impl_{ mgbase::make_unique<impl>(cq, comp_sel) }
{ }

poll_thread::~poll_thread() = default;

} // namespace ibv
} // namespace mgcom

