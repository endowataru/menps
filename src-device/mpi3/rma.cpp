
#include <mgcom.hpp>
#include <mpi.h>

#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/assert.hpp>
#include "impl.hpp"
#include "mpi_base.hpp"

namespace mgcom {

struct mpi3_error { };

class com_mpi3
    : public mpi_base
{
private:
    typedef mgbase::spinlock lock_type;
    
    static const index_t max_num_requests = 16;

public: 
    typedef index_t win_id_t;
    
    void initialize(int* argc, char*** argv)
    {
        mpi_base::initialize(argc, argv);
        
        throw_if_error(
            ::MPI_Win_create_dynamic(MPI_INFO_NULL, MPI_COMM_WORLD, &win_)
        );
        throw_if_error( 
            ::MPI_Win_lock_all(0, win_) // TODO : Assertion
        );
        
        num_requests_ = 0;
    }
    
    void finalize()
    {
        throw_if_error(
            ::MPI_Win_unlock_all(win_)
        );
        
        throw_if_error(
            ::MPI_Win_free(&win_)
        );
        
        mpi_base::finalize();
    }
    
    MPI_Aint attach(void* ptr, ::MPI_Aint size)
    {
        mgbase::lock_guard<lock_type> lc(lock_);
        
        throw_if_error(
            ::MPI_Win_attach(win_, ptr, size)
        );
        
        MPI_Aint addr;
        throw_if_error(
            ::MPI_Get_address(ptr, &addr)
        );
        return addr;
    }
    
    void detach(void* ptr)
    {
        mgbase::lock_guard<lock_type> lc(lock_);
        
        throw_if_error(
            ::MPI_Win_detach(win_, ptr)
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
        if (!lock_.try_lock())
            return false;
        
        mgbase::lock_guard<lock_type> lc(lock_, mgbase::adopt_lock);
        
        if (num_requests_ >= max_num_requests)
            return false;
        
        throw_if_error(
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
        if (!lock_.try_lock())
            return;
        
        mgbase::lock_guard<lock_type> lc(lock_, mgbase::adopt_lock);
        
        throw_if_error(
            ::MPI_Win_flush_all(win_)
        );
        
        for (index_t i = 0; i < num_requests_; ++i)
            notify(on_complete_[i]);
        
        num_requests_ = 0;
    }
    
private:
    void throw_if_error(int err) {
        if (err != MPI_SUCCESS)
            throw mpi3_error();
    }
    
    lock_type lock_;
    MPI_Win win_;
    local_notifier on_complete_[max_num_requests];
    index_t num_requests_;
};

namespace {

com_mpi3 g_com;

}

void initialize(int* argc, char*** argv) {
    g_com.initialize(argc, argv);
}

void finalize() {
    g_com.finalize();
}

local_region register_region(
    void*   local_pointer
,   index_t size_in_bytes
) {
    const ::MPI_Aint addr = g_com.attach(local_pointer, static_cast< ::MPI_Aint>(size_in_bytes));
    return make_local_region(make_region_key(reinterpret_cast<void*>(addr), 0), 0);
}

void deregister_region(const local_region& region)
{
    g_com.detach(to_pointer(region));
}

remote_region use_remote_region(
    process_id_t      /*proc_id*/
,   const region_key& key
) {
    // Do nothing on MPI-3
    return make_remote_region(key, 0 /* unused */);
}

bool try_write_async(
    const local_address&  local_addr
,   const remote_address& remote_addr
,   index_t               size_in_bytes
,   process_id_t          dest_proc
,   local_notifier        on_complete
)
{
    return g_com.try_put(
        to_pointer(local_addr)
    ,   static_cast<int>(dest_proc)
    ,   reinterpret_cast<MPI_Aint>(to_pointer(remote_addr))
    ,   static_cast<int>(size_in_bytes)
    ,   on_complete
    );
}

void poll() {
    g_com.flush();
}

void barrier() {
    ::MPI_Barrier(MPI_COMM_WORLD);
}

process_id_t current_process_id() MGBASE_NOEXCEPT {
    return g_com.current_process_id();
}

index_t number_of_processes() MGBASE_NOEXCEPT {
    return g_com.number_of_processes();
}

}

