
#pragma once

#include <menps/medsm/space.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medsm {

class space_ref
{
public:
    space_ref() = default;
    
    explicit space_ref(space* const sp)
        : sp_(sp)
    { }
    
    space_ref(const space_ref&) = delete;
    space_ref& operator = (const space_ref&) = delete;
    
    space_ref(space_ref&&) noexcept = default;
    space_ref& operator = (space_ref&&) noexcept = default;
    
    segment_ref make_segment(
        const mefdn::size_t    size_in_bytes
    ,   const mefdn::size_t    page_size_in_bytes
    ,   const mefdn::size_t    block_size_in_bytes
    ) {
        return sp_->make_segment(size_in_bytes, page_size_in_bytes, block_size_in_bytes);
    }
    
    void read_barrier() {
        sp_->read_barrier();
    }
    void write_barrier() {
        sp_->write_barrier();
    }
    void async_read_barrier(mefdn::callback<void ()> cb) {
        sp_->async_read_barrier(cb);
    }
    void async_write_barrier(mefdn::callback<void ()> cb) {
        sp_->async_write_barrier(cb);
    }
    
    void enable_on_this_thread()
    {
        sp_->enable_on_this_thread();
    }
    void disable_on_this_thread()
    {
        sp_->disable_on_this_thread();
    }
    
    void pin(void* const ptr, const mefdn::size_t size_in_bytes)
    {
        sp_->pin(ptr, size_in_bytes);
    }
    void unpin(void* const ptr, const mefdn::size_t size_in_bytes)
    {
        sp_->unpin(ptr, size_in_bytes);
    }
    
private:
    mefdn::unique_ptr<space> sp_;
};

} // namespace medsm
} // namespace menps

