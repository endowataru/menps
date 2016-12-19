
#pragma once

#include "sharer/sharer_space.hpp"

namespace mgdsm {

class reconciler
{
public:
    explicit reconciler(sharer_space&);
    
    void write_barrier();
    
    void read_barrier();
    
private:
    
    #if 0
    
    void loop()
    {
        while (!finished())
        {
            // Write all current blocks.
            
            
        }
    }
    
    struct epoch_data
    {
        requester_block*    first_blk;
    };
    
    std::deque<epoch_data> epochs_;
    #endif
};

} // namespace mgdsm

