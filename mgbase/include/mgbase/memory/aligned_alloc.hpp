
#pragma once

#include <mgbase/lang.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/arithmetic.hpp>
#include <cstdlib>
#include <iostream>

namespace mgbase {

struct alloc_error { };

namespace /*unnamed*/ {

inline void* aligned_alloc(std::size_t alignment, std::size_t size) {
    MGBASE_ASSERT(alignment > 0);
    MGBASE_ASSERT(size > 0);
    MGBASE_ASSERT(size % alignment == 0);
    
    void* result;
    /*#ifdef MGBASE_CXX11_SUPPORTED
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
    MGBASE_ASSERT(result != MGBASE_NULLPTR);
    return result;
}

} // unnamed namespace

} // namespace mgbase

