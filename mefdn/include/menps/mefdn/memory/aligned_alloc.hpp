
#pragma once

#include <menps/mefdn/lang.hpp>
#include <menps/mefdn/assert.hpp>
#include <menps/mefdn/arithmetic.hpp>
#include <cstdlib>
#include <iostream>

namespace menps {
namespace mefdn {

struct alloc_error { };

namespace /*unnamed*/ {

inline void* aligned_alloc(std::size_t alignment, std::size_t size) {
    MEFDN_ASSERT(alignment > 0);
    MEFDN_ASSERT(size > 0);
    MEFDN_ASSERT(size % alignment == 0);
    
    void* result;
    /*#ifdef MEFDN_CXX11_SUPPORTED
        result = std::aligned_alloc(alignment, size);
    #else*/
        if (alignment < sizeof(void*)) {
            // Fix the alignment.
            alignment = sizeof(void*);
            size = roundup_divide(size, sizeof(void*)) * sizeof(void*);
        }
        
        // POSIX
        int ret = posix_memalign(&result, alignment, size);
        if (ret != 0)
            throw alloc_error();
    //#endif
    MEFDN_ASSERT(result != nullptr);
    return result;
}

} // unnamed namespace

} // namespace mefdn
} // namespace menps

