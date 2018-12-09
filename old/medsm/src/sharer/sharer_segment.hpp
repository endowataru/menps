
#pragma once

#include "sharer_segment_entry.hpp"
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace menps {
namespace medsm {

class sharer_segment
    : private sharer_segment_entry
{
    typedef sharer_segment_entry     base;
    
public:
    template <typename Conf>
    explicit sharer_segment(Conf&& conf)
        : base(mefdn::forward<Conf>(conf))
    { }
    
    class accessor;
};

} // namespace medsm
} // namespace menps

