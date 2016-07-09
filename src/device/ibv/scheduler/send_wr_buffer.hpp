
#pragma once

#include "device/ibv/native/send_work_request.hpp"
#include <mgbase/scoped_ptr.hpp>

namespace mgcom {
namespace ibv {

class send_wr_buffer
{
public:
    static const mgbase::size_t max_size = 128;
    
    send_wr_buffer()
        : wrs_{new send_work_request[max_size]}
    {
        for (mgbase::size_t i = 0; i < max_size - 1; ++i)
        {
            wrs_[i].next = &wrs_[i + 1];
        }
        
        wrs_[max_size - 1].next = MGBASE_NULLPTR;
    }
    
    send_wr_buffer(const send_wr_buffer&) = delete;
    send_wr_buffer& operator = (const send_wr_buffer&) = delete;
    
    send_work_request& first() {
        return wrs_[0];
    }
    
    send_work_request& at(const mgbase::size_t index)
    {
        MGBASE_ASSERT(index < max_size);
        
        return wrs_[index];
    }
    
    ibv_send_wr* terminate_at(const mgbase::size_t index)
    {
        MGBASE_ASSERT(index < max_size);
        
        const auto ret = wrs_[index].next;
        wrs_[index].next = MGBASE_NULLPTR;
        return ret;
    }
    
    void link_to(const mgbase::size_t index, ibv_send_wr* const next)
    {
        MGBASE_ASSERT(index < max_size);
        
        wrs_[index].next = next;
    }
    
private:
    mgbase::scoped_ptr<send_work_request []> wrs_;
};

} // namespace ibv
} // namespace mgcom

