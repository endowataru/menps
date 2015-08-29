
#pragma once

#include <mgcom.hpp>
#include <mpi.h>

#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/assert.hpp>
#include "impl.hpp"
#include <mpi_base.hpp>

namespace mgcom {

struct mpi3_error
{
    static void check(int err) {
        if (err != MPI_SUCCESS)
            throw mpi3_error();
    }
};

namespace rma {

namespace {

class impl
{
private:
    static const index_t max_num_requests = 16;

public: 
    typedef index_t win_id_t;
    
    void initialize()
    {
        mpi3_error::check(
            MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, &win_)
        );
        mpi3_error::check( 
            MPI_Win_lock_all(0, win_) // TODO : Assertion
        );
        
        num_requests_ = 0;
    }
    
    void finalize()
    {
        mpi3_error::check(
            MPI_Win_unlock_all(win_)
        );
        
        mpi3_error::check(
            MPI_Win_free(&win_)
        );
        
        mpi_base::finalize();
    }
    
    MPI_Aint attach(void* ptr, ::MPI_Aint size)
    {
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        
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
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        
        mpi3_error::check(
            MPI_Win_detach(win_, ptr)
        );
    }
    
    bool try_put(
        const void* src_ptr
    ,   int dest_rank
    ,   ::MPI_Aint dest_index
    ,   int size
    ,   local_notifier on_complete
    )
    {
        if (!mpi_base::get_lock().try_lock())
            return false;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        if (num_requests_ >= max_num_requests)
            return false;
        
        mpi3_error::check(
            MPI_Put(
                src_ptr, size, MPI_BYTE,
                dest_rank, dest_index, size, MPI_BYTE,
                win_
            )
        );
        
        on_complete_[num_requests_++] = on_complete;
        
        return true;
    }
    
    
    void flush()
    {
        if (!mpi_base::get_lock().try_lock())
            return;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        mpi3_error::check(
            ::MPI_Win_flush_all(win_)
        );
        
        for (index_t i = 0; i < num_requests_; ++i)
            notify(on_complete_[i]);
        
        num_requests_ = 0;
    }
    
private:
    MPI_Win win_;
    local_notifier on_complete_[max_num_requests];
    index_t num_requests_;
};

}

}

}

