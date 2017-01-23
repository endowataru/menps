
#pragma once

#include <mgdev/ibv/verbs.hpp>
#include <mgdev/ibv/ibv_error.hpp>
#include <mgbase/assert.hpp>

namespace mgdev {
namespace ibv {

class queue_pair
    : mgbase::noncopyable
{
    static const mgbase::uint32_t max_send_wr = 1 << 14;
    static const mgbase::uint32_t max_recv_wr = 1 << 14;
    static const mgbase::uint32_t max_send_sge = 1; // Scatter/Gather
    static const mgbase::uint32_t max_recv_sge = 1;
    
public:
    queue_pair();
    
    ~queue_pair();
    
    mgbase::uint32_t get_qp_num() const MGBASE_NOEXCEPT {
        return qp_->qp_num;
    }
    
    void create(ibv_context& ctx, ibv_cq& cq, ibv_pd& pd);
    
    void start(
        const mgbase::uint32_t  qp_num
    ,   const mgbase::uint16_t  lid
    ,   const ibv_device_attr&  device_attr
    );
    
    MGBASE_WARN_UNUSED_RESULT
    bool try_post_send(ibv_send_wr& wr, ibv_send_wr** const bad_wr) const
    {
        MGBASE_ASSERT(qp_ != MGBASE_NULLPTR);
        
        const int err = ibv_post_send(qp_, &wr, bad_wr);
        
        if (MGBASE_LIKELY(err == 0)) {
            log_wr("Posted IBV request.", wr);
            return true;
        }
        else if (err == ENOMEM) {
            log_wr("Used up the memory for posting IBV request.", wr);
            return false;
        }
        else {
            log_wr("Failed to post an IBV request.", wr);
            throw ibv_error("ibv_post_send() failed", err);
        }
    }
    
private:
    #ifdef MGBASE_DEBUG
    void log_wr(const char* const msg, const ibv_send_wr& wr) const {
        log_wr_impl(msg, wr);
    }
    #else
    void log_wr(const char* const /*msg*/, const ibv_send_wr& /*wr*/) const { }
    #endif
    
    void log_wr_impl(const char* const msg, const ibv_send_wr& wr) const;
    
private:
    ibv_qp* qp_;
};

} // namespace ibv
} // namespace mgdev

