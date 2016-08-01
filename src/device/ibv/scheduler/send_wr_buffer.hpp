
#pragma once

#include "device/ibv/native/send_work_request.hpp"
#include <mgbase/container/circular_buffer.hpp>

namespace mgcom {
namespace ibv {

class send_wr_buffer
{
public:
    static const mgbase::size_t max_size = 128;
    
    send_wr_buffer()
        : wrs_(max_size)
    {
        for (mgbase::size_t i = 0; i < max_size; ++i)
        {
            auto& wr = raw_at(i);
            wr.next = &next_of(wr);
        }
    }
    
    send_wr_buffer(const send_wr_buffer&) = delete;
    send_wr_buffer& operator = (const send_wr_buffer&) = delete;
    
    send_work_request* try_enqueue(mgbase::size_t* const wr_index_result)
    {
        if (wrs_.full())
            return MGBASE_NULLPTR;
        
        // Important: Don't zero-initialize WR here
        wrs_.push_back();
        
        const auto ret = &wrs_.back();
        
        *wr_index_result = static_cast<mgbase::size_t>(
            ret - &raw_at(0)
        );
        
        return ret;
    }
    
    send_work_request& front() {
        return wrs_.front();
    }
    
    bool empty() const MGBASE_NOEXCEPT {
        return wrs_.empty();
    }
    
    void terminate()
    {
        invariant();
        
        auto& last_wr = wrs_.back();
        
        // Disconnect the last element.
        last_wr.next = MGBASE_NULLPTR;
    }
    
    void relink()
    {
        auto& last_wr = wrs_.back();
        MGBASE_ASSERT(last_wr.next == MGBASE_NULLPTR);
        
        last_wr.next = &next_of(last_wr);
        
        invariant();
    }
    
    void consume(ibv_send_wr* const bad_wr_)
    {
        const auto bad_wr = static_cast<send_work_request*>(bad_wr_);
        
        if (bad_wr == MGBASE_NULLPTR) {
            consume_n(wrs_.size());
            return;
        }
        
        mgbase::ptrdiff_t diff = bad_wr - &front();
        
        const auto num = static_cast<mgbase::size_t>(
            diff >= 0 ? diff : diff + static_cast<mgbase::ptrdiff_t>(max_size)
        );
        
        consume_n(num);
    }
    
private:
    void consume_n(const mgbase::size_t num)
    {
        wrs_.erase_begin(num);
        
        MGBASE_LOG_DEBUG(
            "msg:Consumed IBV requests."
            "num:{}\tsize:{}"
        ,   num
        ,   wrs_.size()
        );
    }
    
    void invariant()
    {
        /*#ifdef MGBASE_DEBUG
        MGBASE_RANGE_BASED_FOR(const auto& wr, wrs_) {
            MGBASE_ASSERT(wr.next != MGBASE_NULLPTR);
        }
        #endif*/
    }
    
    send_work_request& next_of(const send_work_request& wr) MGBASE_NOEXCEPT
    {
        return raw_at((raw_index_of(wr) + 1) % max_size);
    }
    
    mgbase::size_t raw_index_of(const send_work_request& wr) MGBASE_NOEXCEPT
    {
        return static_cast<mgbase::size_t>(&wr - &raw_at(0));
    }
    
    send_work_request& raw_at(const mgbase::size_t index) MGBASE_NOEXCEPT
    {
        MGBASE_ASSERT(index < max_size);
        
        return wrs_.raw_data()[index];
    }
    
    mgbase::circular_buffer<send_work_request> wrs_;
};

} // namespace ibv
} // namespace mgcom

