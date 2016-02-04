
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/threading/spinlock.hpp>

#include "mpi.hpp"

#include "mpi_error.hpp"

namespace mgcom {
namespace mpi {

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

inline bool is_valid_rank(int rank) {
    return valid_process_id(static_cast<process_id_t>(rank));
}

} // namespace mpi

} // namespace mgcom

