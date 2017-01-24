
#pragma once

#include <mgdsm/space.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class space_ref
{
public:
    space_ref() = default;
    
    explicit space_ref(space* const sp)
        : sp_(sp)
    { }
    
    space_ref(const space_ref&) = delete;
    space_ref& operator = (const space_ref&) = delete;
    
    MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_1(space_ref, sp_)
    
    segment_ref make_segment(
        const mgbase::size_t    size_in_bytes
    ,   const mgbase::size_t    page_size_in_bytes
    ,   const mgbase::size_t    block_size_in_bytes
    ) {
        return sp_->make_segment(size_in_bytes, page_size_in_bytes, block_size_in_bytes);
    }
    
    void read_barrier() {
        sp_->read_barrier();
    }
    void write_barrier() {
        sp_->write_barrier();
    }
    
    void enable_on_this_thread()
    {
        sp_->enable_on_this_thread();
    }
    void disable_on_this_thread()
    {
        sp_->disable_on_this_thread();
    }
    
    void pin(void* const ptr, const mgbase::size_t size_in_bytes)
    {
        sp_->pin(ptr, size_in_bytes);
    }
    void unpin(void* const ptr, const mgbase::size_t size_in_bytes)
    {
        sp_->unpin(ptr, size_in_bytes);
    }
    
private:
    mgbase::unique_ptr<space> sp_;
};

} // namespace mgdsm

