
#include "poll_thread.hpp"
#include <menps/mefdn/logger.hpp>
#include "completion_selector.hpp"
#include <menps/medev/ibv/ibv_error.hpp>
#include <menps/mecom/ult.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace mecom {
namespace ibv {

using medev::ibv::completion_queue;
using medev::ibv::ibv_error;

class poll_thread::impl
{
    static const mefdn::size_t max_num_polled = 1 << 12;
    
public:
    impl(completion_queue& cq, completion_selector& comp_sel)
        : finished_{false}
        , cq_(cq)
        , comp_sel_(comp_sel)
        , wcs_(mefdn::make_unique<ibv_wc []>(max_num_polled))
    {
        #if defined(MECOM_IBV_ENABLE_SLEEP_CQ) && defined(MECOM_FORK_COMPLETER_THREAD)
        comp_sel_.set_entrypoint(&impl::loop_start, this);
        #else
        th_ = ult::thread(starter{*this});
        #endif
    }
    
    ~impl()
    {
        finished_ = true;
        
        #ifdef MECOM_IBV_ENABLE_SLEEP_CQ
        comp_sel_.force_notify();
        #endif
        
        #ifndef MECOM_FORK_COMPLETER_THREAD
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
        
        return nullptr;
    }
    
    void loop()
    {
        MEFDN_LOG_DEBUG("msg:Started IBV polling.");
        
        while (MEFDN_LIKELY(!finished_))
        {
            const int ret = cq_.poll(wcs_.get(), max_num_polled);
            
            if (ret > 0)
            {
                const auto num = static_cast<mefdn::size_t>(ret);
                
                for (mefdn::size_t i = 0; i < num; ++i)
                {
                    auto& wc = wcs_[i];
                    
                    if (MEFDN_UNLIKELY(wc.status != IBV_WC_SUCCESS)) {
                        throw ibv_error("polling failed", wc.status);
                    }
                    
                    auto& comp = comp_sel_.get(wc.qp_num);
                    
                    comp.notify(wc.wr_id);
                    
                    MEFDN_LOG_DEBUG(
                        "msg:Polled completion.\t"
                        "wr_id:{}"
                    ,   wc.wr_id
                    );
                }
                
                #ifdef MECOM_IBV_ENABLE_SLEEP_CQ
                #ifdef MECOM_FORK_COMPLETER_THREAD
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
                MEFDN_LOG_VERBOSE("msg:IBV CQ was empty.");
                
                #ifdef MECOM_IBV_ENABLE_SLEEP_CQ
                #ifdef MECOM_FORK_COMPLETER_THREAD
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
        
        MEFDN_LOG_DEBUG("msg:Finished IBV polling.");
    }
    
    bool finished_;
    completion_queue& cq_;
    completion_selector& comp_sel_;
    ult::thread th_;
    mefdn::unique_ptr<ibv_wc []> wcs_;
};

poll_thread::poll_thread(completion_queue& cq, completion_selector& comp_sel)
    : impl_{ mefdn::make_unique<impl>(cq, comp_sel) }
{ }

poll_thread::~poll_thread() = default;

} // namespace ibv
} // namespace mecom
} // namespace menps

