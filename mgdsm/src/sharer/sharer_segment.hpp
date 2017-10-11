
#pragma once

#include "sharer_segment_entry.hpp"
#include <mgbase/unique_ptr.hpp>

namespace mgdsm {

class sharer_segment
    : private sharer_segment_entry
{
    typedef sharer_segment_entry     base;
    
public:
    template <typename Conf>
    explicit sharer_segment(Conf&& conf)
        : base(mgbase::forward<Conf>(conf))
    { }
    
    class accessor;
};

} // namespace mgdsm

