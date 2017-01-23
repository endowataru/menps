
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
    
    #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
    space_ref(space_ref&&) MGBASE_NOEXCEPT_DEFAULT = default;
    #else
    space_ref(space_ref&& other) MGBASE_NOEXCEPT
        : sp_(mgbase::move(other.sp_))
    { }
    #endif
    
    #ifdef MGBASE_CXX11_MOVE_ASSIGNMENT_DEFAULT_SUPPORTED
    space_ref& operator = (space_ref&&) MGBASE_NOEXCEPT_DEFAULT = default;
    #else
    space_ref& operator = (space_ref&& other) MGBASE_NOEXCEPT
    {
        this->sp_ = mgbase::move(other.sp_);
        return *this;
    }
    #endif
    
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

