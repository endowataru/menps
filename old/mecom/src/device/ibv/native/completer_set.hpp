
#pragma once

#include <menps/medev/ibv/completion_queue.hpp>
#include "device/ibv/command/tag_queue.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include "device/ibv/command/poll_thread.hpp"

namespace menps {
namespace mecom {
namespace ibv {

using medev::ibv::completion_queue;

class completer_set
{
public:
    struct config {
        ibv_context&    dev_ctx;
        mefdn::size_t  num_completers;
    };
    
    explicit completer_set(const config& conf)
        : cqs_(mefdn::make_unique<cq_info []>(conf.num_completers))
    {
        for (mefdn::size_t i = 0; i < conf.num_completers; ++i)
        {
            auto& info = cqs_[i];
            
            info.cq = medev::ibv::make_completion_queue(&conf.dev_ctx);
            
            info.poll_th =
                mefdn::make_unique<poll_thread>(
                    info.cq
                ,   info.comp_sel
                );
        }
    }
    
    ~completer_set() /*noexcept*/ = default;
    
    completion_queue& get_cq(const mefdn::size_t i) const noexcept {
        return this->cqs_[i].cq;
    }
    
    void set_qp_num(const mefdn::size_t i, const qp_num_t qp_num, tag_queue& tag_que) {
        this->cqs_[i].comp_sel.set(qp_num, tag_que);
    }
    
    completion_selector& get_comp_sel(const mefdn::size_t i) {
        return this->cqs_[i].comp_sel;
    }
    
private:
    struct cq_info
    {
        completion_queue                cq;
        completion_selector             comp_sel;
        mefdn::unique_ptr<poll_thread> poll_th;
    };
    
    mefdn::unique_ptr<cq_info []> cqs_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

