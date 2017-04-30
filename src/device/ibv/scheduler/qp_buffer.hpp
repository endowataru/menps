
#pragma once

#include "device/ibv/command/completer.hpp"
#include "send_wr_buffer.hpp"
#include "device/ibv/command/set_command_to.hpp"
#include "device/ibv/native/alltoall_queue_pairs.hpp"
#include "device/ibv/command/completion_selector.hpp"

namespace mgcom {
namespace ibv {

class qp_buffer
{
public:
    struct config {
        queue_pair&             qp;
        completer&              tag_que;
        rma::allocator&         alloc;
        completion_selector&    comp_sel;
        bool                    reply_be;
    };
    
    explicit qp_buffer(const config& conf)
        : conf_(conf)
        , sges_(send_wr_buffer::max_size)
        , atomic_buf_(
            atomic_buffer::config{ conf.alloc, completer::max_num_completions, conf.reply_be }
        )
    { }
    
    qp_buffer(const qp_buffer&) = delete;
    qp_buffer& operator = (const qp_buffer&) = delete;
    
    template <typename Command>
    bool try_enqueue(const Command& cmd)
    {
        auto& tag_que = conf_.tag_que;
        
        auto t = tag_que.try_start();
        if (!t.valid()) return false;
        
        const auto wr_id = *t.begin();
        
        mgbase::size_t wr_index = 0;
        const auto wr = wr_buf_.try_enqueue(&wr_index);
        if (MGBASE_UNLIKELY(!wr)) {
            t.rollback();
            return false;
        }
        
        auto& sge = sges_[wr_index];
        
        set_command_to(cmd, wr_id, wr, &sge, tag_que, atomic_buf_);
        
        t.commit();
        
        #ifdef MGCOM_IBV_ENABLE_SLEEP_CQ
        conf_.comp_sel.notify(1);
        #endif
        
        return true;
    }
    
    bool try_post_all()
    {
        MGBASE_ASSERT(!wr_buf_.empty());
        
        wr_buf_.terminate();
        
        ibv_send_wr* bad_wr = MGBASE_NULLPTR;
        
        MGBASE_UNUSED
        const bool success = conf_.qp.try_post_send(wr_buf_.front(), &bad_wr);
        
        wr_buf_.relink();
        
        wr_buf_.consume(bad_wr);
        
        MGBASE_LOG_DEBUG(
            "msg:Posted IBV requests.\t"
            "bad_wr:{:x}"
        ,   reinterpret_cast<mgbase::uintptr_t>(bad_wr)
        );
        
        return bad_wr == MGBASE_NULLPTR;
    }
    
private:
    const config            conf_;
    send_wr_buffer          wr_buf_;
    std::vector<ibv_sge>    sges_;
    atomic_buffer           atomic_buf_;
};

} // namespace ibv
} // namespace mgcom

