
#pragma once

#include "rma.hpp"
#include "common/rma/rma.hpp"
#include "common/notifier.hpp"
#include "common/mpi_base.hpp"

#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/assert.hpp>
#include <mgbase/logging/logger.hpp>

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
    }
    
    MPI_Aint attach(void* ptr, ::MPI_Aint size)
    {
        MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(size > 0);
        
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
        MGBASE_ASSERT(ptr != MGBASE_NULLPTR);
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        
        mpi3_error::check(
            MPI_Win_detach(win_, ptr)
        );
    }
    
    bool try_get(
        void*                   dest_ptr
    ,   int                     src_rank
    ,   MPI_Aint                src_index
    ,   int                     size
    ,   const local_notifier&   on_complete
    )
    {
        MGBASE_ASSERT(dest_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(is_valid_rank(src_rank));
        MGBASE_ASSERT(size > 0);
        
        if (!mpi_base::get_lock().try_lock())
            return false;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        if (requests_saturated())
            return false;
        
        MGBASE_LOG_DEBUG("msg:RDMA Get.\tdest_ptr:{}\tsrc_rank:{}\tsrc_index:{:x}", dest_ptr, src_rank, src_index);
        
        mpi3_error::check(
            MPI_Get(
                dest_ptr, size, MPI_BYTE,
                src_rank, src_index, size, MPI_BYTE,
                win_
            )
        );
        
        add_notifier(on_complete);
        
        return true;
    }
    
    
    bool try_put(
        const void*             src_ptr
    ,   int                     dest_rank
    ,   MPI_Aint                dest_index
    ,   int                     size
    ,   const local_notifier&   on_complete
    )
    {
        MGBASE_ASSERT(src_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(is_valid_rank(dest_rank));
        MGBASE_ASSERT(size > 0);
        
        if (!mpi_base::get_lock().try_lock())
            return false;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        if (requests_saturated())
            return false;
        
        MGBASE_LOG_DEBUG("msg:RDMA Put.\tsrc_ptr:{}\tdest_rank:{}\tdest_index:{:x}", src_ptr, dest_rank, dest_index);
        
        mpi3_error::check(
            MPI_Put(
                src_ptr, size, MPI_BYTE,
                dest_rank, dest_index, size, MPI_BYTE,
                win_
            )
        );
        
        add_notifier(on_complete);
        
        return true;
    }
    
    bool try_compare_and_swap(
        const void*             expected_ptr
    ,   const void*             desired_ptr
    ,   void*                   result_ptr
    ,   MPI_Datatype            datatype
    ,   int                     dest_rank
    ,   MPI_Aint                dest_index
    ,   const local_notifier&   on_complete
    )
    {
        if (!mpi_base::get_lock().try_lock())
            return false;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        if (requests_saturated())
            return false;
        
        /*
            TODO: const_cast is needed for OpenMPI 1.8.4.
        */
        
        MGBASE_LOG_DEBUG("msg:RDMA CAS.\tdest_rank:{}\tdest_index:{:x}", dest_rank, dest_index);
        
        mpi3_error::check(
            MPI_Compare_and_swap(
                const_cast<void*>(desired_ptr)  // origin_addr
            ,   const_cast<void*>(expected_ptr) // compare_addr
            ,   result_ptr                      // result_addr
            ,   datatype                        // datatype
            ,   dest_rank                       // target_rank
            ,   dest_index                      // target_disp
            ,   win_                            // win
            )
        );
        
        add_notifier(on_complete);
        
        return true;
    }
    
    bool try_fetch_and_op(
        const void*             value_ptr
    ,   void*                   result_ptr
    ,   MPI_Datatype            datatype
    ,   int                     dest_rank
    ,   MPI_Aint                dest_index
    ,   MPI_Op                  operation
    ,   const local_notifier&   on_complete
    )
    {
        MGBASE_ASSERT(value_ptr  != MGBASE_NULLPTR);
        MGBASE_ASSERT(result_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(is_valid_rank(dest_rank));
        
        if (!mpi_base::get_lock().try_lock())
            return false;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        MGBASE_LOG_DEBUG("msg:RDMA Fetch and add.\tdest_rank:{}\tdest_index:{:x}", dest_rank, dest_index);
        
        if (requests_saturated())
            return false;
        
        /*
            TODO: const_cast is needed for OpenMPI 1.8.4.
        */
        
        mpi3_error::check(
            MPI_Fetch_and_op(
                const_cast<void*>(value_ptr)    // origin_addr
            ,   const_cast<void*>(result_ptr)   // result_addr
            ,   datatype                        // datatype
            ,   dest_rank                       // target_rank
            ,   dest_index                      // target_disp
            ,   operation                       // op
            ,   win_                            // win
            )
        );
        
        add_notifier(on_complete);
        
        return true;
    }
    
    void flush()
    {
        if (!mpi_base::get_lock().try_lock())
            return;
        
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
        
        mpi3_error::check(
            MPI_Win_flush_all(win_)
        );
        
        for (index_t i = 0; i < num_requests_; ++i)
            notify(on_complete_[i]);
        
        num_requests_ = 0;
    }
    
private:
    static bool is_valid_rank(int rank) MGBASE_NOEXCEPT {
        return 0 <= rank && rank < static_cast<int>(number_of_processes());
    }
    
    bool requests_saturated() const MGBASE_NOEXCEPT {
        return num_requests_ >= max_num_requests;
    }
    
    void add_notifier(const local_notifier& on_complete) MGBASE_NOEXCEPT {
        on_complete_[num_requests_++] = on_complete;
    }
    
    MPI_Win win_;
    local_notifier on_complete_[max_num_requests];
    index_t num_requests_;
};

}

}

}

