
#pragma once

#include <mgbase/lang.hpp>

namespace mgbase {

#if defined(MGBASE_ARCH_INTEL)

inline void hard_read_memory_barrier() {
    asm volatile ("lfence");
}
inline void hard_write_memory_barrier() {
    asm volatile ("sfence");
}
inline void hard_memory_barrier() {
    asm volatile ("mfence");
}

#elif defined(MGBASE_ARCH_SPARC)

inline void hard_memory_barrier() {
    asm volatile ("membar 15");
}

#endif

inline void soft_memory_barrier() {
    asm volatile ("" ::: "memory");
}

}

