
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/threading/spinlock.hpp>

#include "mpi.hpp"

#include "mpi_error.hpp"

namespace mgcom {
namespace mpi {

void initialize(int* argc, char*** argv);

void finalize();

#if 0

struct mpi_lock
{
    static void lock();
    
    static bool try_lock();
    
    static void unlock();
};

typedef mpi_lock    lock_type;

#endif


#if 0

inline lock_type& get_lock() MGBASE_NOEXCEPT {
    static mpi_lock lc;
    return lc;
}


void native_barrier();

#endif

namespace /*unnamed*/ {

inline bool is_valid_rank(const int rank)
{
    return valid_process_id(static_cast<process_id_t>(rank));
}

} // unnamed namespace

} // namespace mpi

} // namespace mgcom

