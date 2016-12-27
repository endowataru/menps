
#pragma once

#include <mgdsm/space.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class space_ref
{
public:
    explicit space_ref(space* const sp)
        : sp_(sp)
    { }
    
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
    
    
private:
    mgbase::unique_ptr<space> sp_;
};

} // namespace mgdsm

