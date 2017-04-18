
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
        alltoall_queue_pairs&   qps;
        rma::allocator&         alloc;
        completion_selector&    comp_sel;
        process_id_t            proc;
        bool                    reply_be;
    };
    
    explicit qp_buffer(const config& conf)
        : conf_(conf)
        , comp_()
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
        auto t = comp_.try_start();
        if (!t.valid()) return false;
        
        const auto wr_id = *t.begin();
        
        mgbase::size_t wr_index = 0;
        const auto wr = wr_buf_.try_enqueue(&wr_index);
        if (!wr) {
            t.rollback();
            return false;
        }
        
        auto& sge = sges_[wr_index];
        
        set_command_to(cmd, wr_id, wr, &sge, comp_, atomic_buf_);
        
        t.commit();
        
        #ifdef MGCOM_IBV_ENABLE_SLEEP
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
        const bool success = conf_.qps.try_post_send(conf_.proc, 0, wr_buf_.front(), &bad_wr);
        
        wr_buf_.relink();
        
        wr_buf_.consume(bad_wr);
        
        MGBASE_LOG_DEBUG(
            "msg:Posted IBV requests.\t"
            "proc:{}\tbad_wr:{:x}"
        ,   conf_.proc
        ,   reinterpret_cast<mgbase::uintptr_t>(bad_wr)
        );
        
        return bad_wr == MGBASE_NULLPTR;
    }
    
    completer& get_completer() MGBASE_NOEXCEPT
    {
        return comp_;
    }
    
private:
    const config            conf_;
    completer               comp_;
    send_wr_buffer          wr_buf_;
    std::vector<ibv_sge>    sges_;
    atomic_buffer           atomic_buf_;
};

} // namespace ibv
} // namespace mgcom

