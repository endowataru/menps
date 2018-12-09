
#pragma once

#include <menps/medev/ibv/verbs.hpp>
#include <menps/mefdn/container/circular_buffer.hpp>

namespace menps {
namespace mecom {
namespace ibv {

class send_wr_buffer
{
    typedef ibv_send_wr     send_wr_type;
    
public:
    static const mefdn::size_t max_size = 128;
    
    send_wr_buffer()
        : wrs_(max_size)
    {
        for (mefdn::size_t i = 0; i < max_size; ++i)
        {
            auto& wr = raw_at(i);
            wr.next = &next_of(wr);
        }
    }
    
    send_wr_buffer(const send_wr_buffer&) = delete;
    send_wr_buffer& operator = (const send_wr_buffer&) = delete;
    
    send_wr_type* try_enqueue(mefdn::size_t* const wr_index_result)
    {
        if (wrs_.full())
            return nullptr;
        
        // Important: Don't zero-initialize WR here
        wrs_.push_back();
        
        const auto ret = &wrs_.back();
        
        *wr_index_result = static_cast<mefdn::size_t>(
            ret - &raw_at(0)
        );
        
        return ret;
    }
    
    send_wr_type& front() {
        return wrs_.front();
    }
    
    bool empty() const noexcept {
        return wrs_.empty();
    }
    
    void terminate()
    {
        invariant();
        
        auto& last_wr = wrs_.back();
        
        // Disconnect the last element.
        last_wr.next = nullptr;
    }
    
    void relink()
    {
        auto& last_wr = wrs_.back();
        MEFDN_ASSERT(last_wr.next == nullptr);
        
        last_wr.next = &next_of(last_wr);
        
        invariant();
    }
    
    void consume(ibv_send_wr* const bad_wr_)
    {
        const auto bad_wr = static_cast<send_wr_type*>(bad_wr_);
        
        if (bad_wr == nullptr) {
            consume_n(wrs_.size());
            return;
        }
        
        mefdn::ptrdiff_t diff = bad_wr - &front();
        
        const auto num = static_cast<mefdn::size_t>(
            diff >= 0 ? diff : diff + static_cast<mefdn::ptrdiff_t>(max_size)
        );
        
        consume_n(num);
    }
    
private:
    void consume_n(const mefdn::size_t num)
    {
        wrs_.erase_begin(num);
        
        MEFDN_LOG_DEBUG(
            "msg:Consumed IBV requests."
            "num:{}\tsize:{}"
        ,   num
        ,   wrs_.size()
        );
    }
    
    void invariant()
    {
        /*#ifdef MEFDN_DEBUG
        MEFDN_RANGE_BASED_FOR(const auto& wr, wrs_) {
            MEFDN_ASSERT(wr.next != nullptr);
        }
        #endif*/
    }
    
    send_wr_type& next_of(const send_wr_type& wr) noexcept
    {
        return raw_at((raw_index_of(wr) + 1) % max_size);
    }
    
    mefdn::size_t raw_index_of(const send_wr_type& wr) noexcept
    {
        return static_cast<mefdn::size_t>(&wr - &raw_at(0));
    }
    
    send_wr_type& raw_at(const mefdn::size_t index) noexcept
    {
        MEFDN_ASSERT(index < max_size);
        
        return wrs_.raw_data()[index];
    }
    
    mefdn::circular_buffer<send_wr_type> wrs_;
};

} // namespace ibv
} // namespace mecom
} // namespace menps

