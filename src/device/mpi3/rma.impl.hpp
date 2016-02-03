
#pragma once

#include "rma.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "common/rma/rma.hpp"

#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/logging/logger.hpp>

#include <string>

namespace mgcom {
namespace rma {

namespace /*unnamed*/ {

class mpi3_rma
{
private:
    static const index_t max_num_requests = 16;

public: 
    void initialize()
    {
        mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
        
        mpi3_error::check(
            MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, &win_)
        );
        mpi3_error::check( 
            MPI_Win_lock_all(0, win_) // TODO : Assertion
        );
    }
    
    void finalize()
    {
        mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
        
        mpi3_error::check(
            MPI_Win_unlock_all(win_)
        );
        
        mpi3_error::check(
            MPI_Win_free(&win_)
        );
    }
    
    MPI_Aint attach(void* ptr, MPI_Aint size)
    {
        MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(size > 0);
        
        mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
        
        MGBASE_LOG_DEBUG(
            "msg:Attach a memory region.\tptr:{:x}\tsize:{}"
        ,   reinterpret_cast<mgbase::uint64_t>(ptr)
        ,   size
        );
        
        mpi3_error::check(
            MPI_Win_attach(win_, ptr, size)
        );
        
        MPI_Aint addr;
        mpi3_error::check(
            MPI_Get_address(ptr, &addr)
        );
        return addr;
    }
    
    void detach(void* ptr)
    {
        MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
        
        mgbase::lock_guard<mpi::lock_type> lc(mpi::get_lock());
        
        mpi3_error::check(
            MPI_Win_detach(win_, ptr)
        );
    }
    
    MPI_Win get_win() const MGBASE_NOEXCEPT { return win_; }

private:
    MPI_Win win_;
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

