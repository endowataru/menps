
#pragma once

#include <mgbase/lang.hpp>
#include <cstdlib>

namespace mgbase {

namespace {

inline void* aligned_alloc(std::size_t alignment, std::size_t size) {
    #ifdef MGBASE_CPP11_SUPPORTED
        return std::aligned_alloc(alignment, size);
    #else
        // POSIX
        void* result;
        posix_memalign(&result, alignment, size);
        return result;
    #endif
}

}

}

