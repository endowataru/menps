
#pragma once

#include "sharer_page_entry.hpp"

namespace mgdsm {

class sharer_page
    : private sharer_page_entry
{
public:
    class accessor;
    
    // TODO: breaking the safety
    //  This "friend" is needed for set_block_size and set_num_blocks.
    //  Both members are called before accessor classes can be constructed.
    template <typename Policy>
    friend class basic_sharer_segment_entry;
};

} // namespace mgdsm

