
#pragma once

#include <mgdev/ibv/completion_queue.hpp>
#include "device/ibv/command/tag_queue.hpp"
#include "device/ibv/command/completion_selector.hpp"
#include "device/ibv/command/poll_thread.hpp"

namespace mgcom {
namespace ibv {

using mgdev::ibv::completion_queue;

class completer_set
{
public:
    struct config {
        ibv_context&    dev_ctx;
        mgbase::size_t  num_completers;
    };
    
    explicit completer_set(const config& conf)
        : cqs_(mgbase::make_unique<cq_info []>(conf.num_completers))
    {
        for (mgbase::size_t i = 0; i < conf.num_completers; ++i)
        {
            auto& info = cqs_[i];
            
            info.cq = mgdev::ibv::make_completion_queue(&conf.dev_ctx);
            
            info.poll_th =
                mgbase::make_unique<poll_thread>(
                    info.cq
                ,   info.comp_sel
                );
        }
    }
    
    ~completer_set() /*noexcept*/ = default;
    
    completion_queue& get_cq(const mgbase::size_t i) const MGBASE_NOEXCEPT {
        return this->cqs_[i].cq;
    }
    
    void set_qp_num(const mgbase::size_t i, const qp_num_t qp_num, tag_queue& tag_que) {
        this->cqs_[i].comp_sel.set(qp_num, tag_que);
    }
    
    completion_selector& get_comp_sel(const mgbase::size_t i) {
        return this->cqs_[i].comp_sel;
    }
    
private:
    struct cq_info
    {
        completion_queue                cq;
        completion_selector             comp_sel;
        mgbase::unique_ptr<poll_thread> poll_th;
    };
    
    mgbase::unique_ptr<cq_info []> cqs_;
};

} // namespace ibv
} // namespace mgcom

