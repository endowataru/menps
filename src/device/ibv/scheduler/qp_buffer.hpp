
#pragma once

#include "device/ibv/command/completer.hpp"
#include "send_wr_buffer.hpp"
#include "device/ibv/command/set_command_to.hpp"
#include "device/ibv/native/scatter_gather_entry.hpp"
#include <mgbase/ult/this_thread.hpp>
#include "device/ibv/native/endpoint.hpp"

namespace mgcom {
namespace ibv {

class qp_buffer
{
public:
    qp_buffer(ibv::endpoint& ep, rma::allocator& alloc, completer& comp, const process_id_t proc)
        : ep_(ep)
        , proc_(proc)
        , comp_(comp)
        , sges_(send_wr_buffer::max_size)
        , atomic_buf_(alloc, completer::max_num_completions)
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
        
        return true;
    }
    
    
    bool try_post_all()
    {
        MGBASE_ASSERT(!wr_buf_.empty());
        
        wr_buf_.terminate();
        
        ibv_send_wr* bad_wr = MGBASE_NULLPTR;
        
        while (MGBASE_UNLIKELY(
            ! ep_.try_post_send(proc_, wr_buf_.front(), &bad_wr)
        ))
        {
            mgbase::ult::this_thread::yield();
        }
        
        wr_buf_.relink();
        
        wr_buf_.consume(bad_wr);
        
        MGBASE_LOG_DEBUG(
            "msg:Posted IBV requests.\t"
            "proc:{}\tbad_wr:{:x}"
        ,   proc_
        ,   reinterpret_cast<mgbase::uintptr_t>(bad_wr)
        );
        
        return bad_wr == MGBASE_NULLPTR;
    }
    
private:
    ibv::endpoint& ep_;
    process_id_t proc_;
    
    completer& comp_;
    send_wr_buffer wr_buf_;
    std::vector<scatter_gather_entry> sges_;
    atomic_buffer atomic_buf_;
};

} // namespace ibv
} // namespace mgcom

