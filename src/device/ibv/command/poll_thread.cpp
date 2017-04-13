
#include "poll_thread.hpp"
#include "completer.hpp"
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
    {
        th_ = ult::thread(starter{*this});
    }
    
    ~impl()
    {
        finished_ = true;
        
        th_.join();
    }
    
private:
    struct starter
    {
        impl&   self;
        
        void operator() () const {
            self.loop();
        }
    };
    
    void loop()
    {
        MGBASE_LOG_DEBUG("msg:Started IBV polling.");
        
        const auto wcs = mgbase::make_unique<ibv_wc []>(max_num_polled);
        
        while (MGBASE_LIKELY(!finished_))
        {
            const int ret = cq_.poll(wcs.get(), max_num_polled);
            
            if (ret > 0)
            {
                const auto num = static_cast<mgbase::size_t>(ret);
                
                for (mgbase::size_t i = 0; i < num; ++i)
                {
                    auto& wc = wcs[i];
                    
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
    completion_selector& comp_sel_;
    ult::thread th_;
};

poll_thread::poll_thread(completion_queue& cq, completion_selector& comp_sel)
    : impl_{ mgbase::make_unique<impl>(cq, comp_sel) }
{ }

poll_thread::~poll_thread() = default;

} // namespace ibv
} // namespace mgcom

