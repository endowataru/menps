
#pragma once

#error "No longer used"

#include <menps/mefdn/profiling/clock.hpp>
#include <menps/mefdn/operation.hpp>
#include <limits>

namespace menps {
namespace mecom {

class fjmpi_outstanding_list
{
public:
    struct element
    {
        element*            prev;
        element*            next;
        mefdn::cpu_clock_t predicted;
    };
    
    fjmpi_outstanding_list()
    {
        first_.predicted    = 0;
        first_.prev         = nullptr;
        first_.next         = &last_;
        
        last_.predicted     = std::numeric_limits<mefdn::cpu_clock_t>::max();
        last_.prev          = &first_;
        last_.next          = nullptr;
    }
    
    void add(
        element* const              elem
    ,   const mefdn::cpu_clock_t   predicted_cycles
    ) {
        const mefdn::cpu_clock_t predicted_clock
            = mefdn::get_cpu_clock() + predicted_cycles;
        
        // Reversely search the list from its last element.
        element* next = &last_;
        element* prev = last_.prev;
        
        MEFDN_ASSERT(prev != nullptr);
        
        // The key of the first sentinel always is 0.
        // This condition does not hold on it.
        while (prev->predicted > predicted_clock)
        {
            // Move back to the previous element.
            next = prev;
            prev = prev->prev;
            
            MEFDN_ASSERT(prev != nullptr);
        }
        
        MEFDN_ASSERT(prev->prev != nullptr || prev == &first_);
        MEFDN_ASSERT(next->next != nullptr || next == &last_);
        
        MEFDN_ASSERT(prev->next == next);
        MEFDN_ASSERT(next->prev == prev);
        
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
        
        MEFDN_ASSERT(prev != nullptr);
        MEFDN_ASSERT(next != nullptr);
        
        prev->next = next;
        next->prev = prev;
    }
    
    element* try_fetch()
    {
        element* const elem = first_.next;
        
        if (elem == &last_)
            return nullptr;
        
        MEFDN_ASSERT(elem->next != nullptr);
        
        if (elem->predicted <= mefdn::get_cpu_clock())
            return elem;
        else
            return nullptr;
    }
    
private:
    // The first sentinel
    element first_;
    
    // The last sentinel
    element last_;
};

} // namespace mecom
} // namespace menps


