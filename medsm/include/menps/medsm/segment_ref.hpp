
#pragma once

#include <menps/medsm/segment.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medsm {

class segment_ref
{
public: 
    segment_ref() noexcept = default;

    explicit segment_ref(segment* const seg)
        : seg_(seg)
    { }
    
    segment_ref(const segment_ref&) = delete;
    segment_ref& operator = (const segment_ref&) = delete;
    
    segment_ref(segment_ref&&) noexcept = default;
    segment_ref& operator = (segment_ref&&) noexcept = default;
    
    void* get_ptr() const noexcept
    {
        return seg_->get_ptr();
    }
    
    mefdn::size_t get_size_in_bytes() const noexcept
    {
        return seg_->get_size_in_bytes();
    }
    
private:
    mefdn::unique_ptr<segment> seg_;
};

} // namespace medsm
} // namespace menps

