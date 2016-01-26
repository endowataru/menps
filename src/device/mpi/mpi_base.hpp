
#pragma once

#include <mgcom.hpp>
#include <mpi.h>
#include <mgbase/threading/spinlock.hpp>

#include "mpi_error.hpp"

namespace mgcom {
namespace mpi_base {

void initialize(int* argc, char*** argv);

void finalize();

struct mpi_lock
{
    static void lock();
    
    static bool try_lock();
    
    static void unlock();
};

typedef mpi_lock    lock_type;

namespace /*unnamed*/ {

inline lock_type& get_lock() MGBASE_NOEXCEPT {
    static mpi_lock lc;
    return lc;
}

} // unnamed namespace

void native_barrier();

/*typedef mgbase::spinlock  lock_type;

inline lock_type& get_lock() MGBASE_NOEXCEPT
{
    static lock_type lc;
    return lc;
}*/

} // namespace mpi_base
} // namespace mgcom

