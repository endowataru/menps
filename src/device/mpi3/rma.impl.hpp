
#pragma once

#include "rma.hpp"
#include "device/mpi/mpi_base.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "common/rma/rma.hpp"
#include "common/command/comm_call.hpp"

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

private:
    static void initialize_on_this_thread(mpi3_rma& self)
    {
        mpi3_error::check(
            MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, &self.win_)
        );
        mpi3_error::check( 
            MPI_Win_lock_all(0, self.win_) // TODO : Assertion
        );
    }
    
public:
    void initialize()
    {
        mgcom::comm_call<void>(
            mgbase::bind_ref1(
                MGBASE_MAKE_INLINED_FUNCTION(&mpi3_rma::initialize_on_this_thread)
            ,   *this
            )
        );
    }
    
private:
    static void finalize_on_this_thread(mpi3_rma& self)
    {
        mpi3_error::check(
            MPI_Win_unlock_all(self.win_)
        );
        
        mpi3_error::check(
            MPI_Win_free(&self.win_)
        );
    }
    
public:
    void finalize()
    {
        mgcom::comm_call<void>(
            mgbase::bind_ref1(
                MGBASE_MAKE_INLINED_FUNCTION(&mpi3_rma::finalize_on_this_thread)
            ,   *this
            )
        );
    }
    
private:
    struct attach_closure
    {
        MPI_Aint operator() ()
        {
            MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
            MGBASE_ASSERT(size > 0);
            
            MGBASE_LOG_DEBUG(
                "msg:Attach a memory region.\tptr:{:x}\tsize:{}"
            ,   reinterpret_cast<mgbase::uint64_t>(ptr)
            ,   size
            );
            
            mpi3_error::check(
                MPI_Win_attach(self->win_, ptr, size)
            );
            
            MPI_Aint addr;
            mpi3_error::check(
                MPI_Get_address(ptr, &addr)
            );
            
            return addr;
        }
        
        mpi3_rma*   self;
        void*       ptr;
        MPI_Aint    size;
    };
    
public:
    MPI_Aint attach(void* ptr, MPI_Aint size)
    {
        const attach_closure cl = { this, ptr, size };
        return mgcom::comm_call<MPI_Aint>(cl);
    }
    
private:
    struct detach_closure
    {
        void operator () ()
        {
            MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
            
            mpi3_error::check(
                MPI_Win_detach(self->win_, ptr)
            );
        }
        
        mpi3_rma*   self;
        void*       ptr;
    };
    
public:
    void detach(void* ptr)
    {
        const detach_closure cl = { this, ptr };
        mgcom::comm_call<void>(cl);
    }
    
    MPI_Win get_win() const MGBASE_NOEXCEPT { return win_; }

private:
    MPI_Win win_;
};

} // unnamed namespace

} // namespace rma
} // namespace mgcom

