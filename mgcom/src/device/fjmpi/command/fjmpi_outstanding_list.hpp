
#pragma once

#error "No longer used"

#include <mgbase/profiling/clock.hpp>
#include <mgbase/operation.hpp>
#include <limits>

namespace mgcom {

class fjmpi_outstanding_list
{
public:
    struct element
    {
        element*            prev;
        element*            next;
        mgbase::cpu_clock_t predicted;
    };
    
    fjmpi_outstanding_list()
    {
        first_.predicted    = 0;
        first_.prev         = MGBASE_NULLPTR;
        first_.next         = &last_;
        
        last_.predicted     = std::numeric_limits<mgbase::cpu_clock_t>::max();
        last_.prev          = &first_;
        last_.next          = MGBASE_NULLPTR;
    }
    
    void add(
        element* const              elem
    ,   const mgbase::cpu_clock_t   predicted_cycles
    ) {
        const mgbase::cpu_clock_t predicted_clock
            = mgbase::get_cpu_clock() + predicted_cycles;
        
        // Reversely search the list from its last element.
        element* next = &last_;
        element* prev = last_.prev;
        
        MGBASE_ASSERT(prev != MGBASE_NULLPTR);
        
        // The key of the first sentinel always is 0.
        // This condition does not hold on it.
        while (prev->predicted > predicted_clock)
        {
            // Move back to the previous element.
            next = prev;
            prev = prev->prev;
            
            MGBASE_ASSERT(prev != MGBASE_NULLPTR);
        }
        
        MGBASE_ASSERT(prev->prev != MGBASE_NULLPTR || prev == &first_);
        MGBASE_ASSERT(next->next != MGBASE_NULLPTR || next == &last_);
        
        MGBASE_ASSERT(prev->next == next);
        MGBASE_ASSERT(next->prev == prev);
        
        // Insert a new element to the searched position.
        // We will change
        //      [prev] <-> [next]
        // to
        //      [prev] <-> [elem] <-> [next]
        
        elem->predicted = predicted_clock;
        elem->prev = prev;
        elem->next = next;
        
        prev->next = elem;
        next->prev = elem;
    }
    
    void remove(element* const elem)
    {
        // Remove the element at the specified index.
        element* const prev = elem->prev;
        element* const next = elem->next;
        
        MGBASE_ASSERT(prev != MGBASE_NULLPTR);
        MGBASE_ASSERT(next != MGBASE_NULLPTR);
        
        prev->next = next;
        next->prev = prev;
    }
    
    element* try_fetch()
    {
        element* const elem = first_.next;
        
        if (elem == &last_)
            return MGBASE_NULLPTR;
        
        MGBASE_ASSERT(elem->next != MGBASE_NULLPTR);
        
        if (elem->predicted <= mgbase::get_cpu_clock())
            return elem;
        else
            return MGBASE_NULLPTR;
    }
    
private:
    // The first sentinel
    element first_;
    
    // The last sentinel
    element last_;
};

} // namespace mgcom


