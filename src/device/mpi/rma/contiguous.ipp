
#pragma once

#include <mgcom/am.hpp>
#include <mgcom/rma.hpp>
#include <common/rma/rma.hpp>
#include <common/mpi_base.hpp>
#include <common/mpi_error.hpp>
#include <device/mpi/am/am.hpp>

#include <mgbase/threading/lock_guard.hpp>

namespace mgcom {
namespace rma {
namespace untyped {

namespace /*unnamed*/ {

class emulated_contiguous
{
    MGBASE_STATIC_ASSERT(sizeof(void*) >= sizeof(MPI_Request), "handle size must be larger than MPI_Request");
    
public:
    emulated_contiguous()
        : tag_(0) { }
    
    void initialize()
    {
        mgcom::am::register_roundtrip_handler<am_read>();
        mgcom::am::register_roundtrip_handler<am_write>();
    }

private:
    class am_read
    {
    public:
        static const mgcom::am::handler_id_t request_id = 2100; // TODO: remove magic numbers
        static const mgcom::am::handler_id_t reply_id   = 2101;
        
        struct argument_type {
            int         tag;
            const void* src_ptr;
            index_t     size_in_bytes;
        };
        typedef void    return_type;
        
        static return_type on_request(
            const mgcom::am::callback_parameters& params
        ,   const argument_type& arg
        ) {
            mpi_error::check(
                MPI_Send(
                    const_cast<void*>(arg.src_ptr) // For old MPI implementations
                ,   arg.size_in_bytes
                ,   MPI_BYTE
                ,   params.source
                ,   arg.tag
                ,   MPI_COMM_WORLD
                )
            );
        }
    };
    
public:
    template <emulated_contiguous& self>
    class read_handlers
    {
        typedef remote_read_cb          cb_type;
        typedef mgbase::deferred<void>  result_type;
        typedef result_type (func_type)(cb_type&);
        
    public:
        static result_type start(cb_type& cb)
        {
            cb.tag = self.new_tag();
            return try_recv(cb);
        }
    
    private:
        static result_type try_recv(cb_type& cb)
        {
            if (!mpi_base::get_lock().try_lock())
                return mgbase::make_deferred<func_type, &try_recv>(cb);
            
            {
                mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
                
                void* const dest_ptr = mgcom::rma::untyped::to_pointer(cb.local_addr);
                
                MPI_Request* const request = reinterpret_cast<MPI_Request*>(&cb.request);
                
                mpi_error::check(
                    MPI_Irecv(
                        dest_ptr
                    ,   cb.size_in_bytes
                    ,   MPI_BYTE
                    ,   cb.proc
                    ,   cb.tag
                    ,   MPI_COMM_WORLD
                    ,   request
                    )
                );
            }
            
            void* const src_ptr = mgcom::rma::untyped::to_pointer(cb.remote_addr);
            
            const am_read::argument_type arg = { cb.tag, src_ptr, cb.size_in_bytes };
            
            return mgbase::add_continuation<func_type, test>(
                cb
            ,   mgcom::am::call_roundtrip_nb<am_read>(
                    cb.cb_roundtrip
                ,   cb.proc
                ,   arg
                )
            );
        }
        
        static result_type test(cb_type& cb)
        {
            MPI_Request* const request = reinterpret_cast<MPI_Request*>(&cb.request);
            
            int flag;
            MPI_Status status;
            mpi_error::check(
                MPI_Test(request, &flag, &status)
            );
            
            if (flag)
                return mgbase::make_ready_deferred();
            else
                return mgbase::make_deferred<func_type, test>(cb);
        }
    };
    
private:
    class am_write
    {
    public:
        static const mgcom::am::handler_id_t request_id = 2102;
        static const mgcom::am::handler_id_t reply_id   = 2103;
        
        struct argument_type {
            int         tag;
            const void* dest_ptr;
            index_t     size_in_bytes;
        };
        typedef void    return_type;
        
        static return_type on_request(
            const mgcom::am::callback_parameters& params
        ,   const argument_type& arg
        ) {
            MPI_Status status;
            mpi_error::check(
                MPI_Recv(
                    const_cast<void*>(arg.dest_ptr) // For old MPI implementations
                ,   arg.size_in_bytes
                ,   MPI_BYTE
                ,   params.source
                ,   arg.tag
                ,   MPI_COMM_WORLD
                ,   &status
                )
            );
        }
    };
    
public:
    template <emulated_contiguous& self>
    class write_handlers
    {
        typedef remote_write_cb         cb_type;
        typedef mgbase::deferred<void>  result_type;
        typedef result_type (func_type)(cb_type&);
        
    public:
        static result_type start(cb_type& cb)
        {
            cb.tag = self.new_tag();
            return try_send(cb);
        }
    
    private:
        static result_type try_send(cb_type& cb)
        {
            if (!mpi_base::get_lock().try_lock())
                return mgbase::make_deferred<func_type, &try_send>(cb);
            
            {
                mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock(), mgbase::adopt_lock);
                
                const void* const src_ptr = mgcom::rma::untyped::to_pointer(cb.local_addr);
                
                MPI_Request* const request = reinterpret_cast<MPI_Request*>(&cb.request);
                
                mpi_error::check(
                    MPI_Isend(
                        const_cast<void*>(src_ptr) // For old MPI implementations
                    ,   cb.size_in_bytes
                    ,   MPI_BYTE
                    ,   cb.proc
                    ,   cb.tag
                    ,   MPI_COMM_WORLD
                    ,   request
                    )
                );
            }
            
            void* const dest_ptr = mgcom::rma::untyped::to_pointer(cb.remote_addr);
            
            const am_write::argument_type arg = { cb.tag, dest_ptr, cb.size_in_bytes };
            
            return mgbase::add_continuation<func_type, test>(
                cb
            ,   mgcom::am::call_roundtrip_nb<am_write>(
                    cb.cb_roundtrip
                ,   cb.proc
                ,   arg
                )
            );
        }
        
        static result_type test(cb_type& cb)
        {
            MPI_Request* const request = reinterpret_cast<MPI_Request*>(&cb.request);
            
            int flag;
            MPI_Status status;
            mpi_error::check(
                MPI_Test(request, &flag, &status)
            );
            
            if (flag)
                return mgbase::make_ready_deferred();
            else
                return mgbase::make_deferred<func_type, test>(cb);
        }
    };
    

private:
    int new_tag()
    {
        // TODO: limit of int (not a serious problem though)
        int tag = ++tag_;
        
        if (tag == am::get_tag())
            return new_tag();
        else
            return tag;
    }
    
    int tag_;
};

} // unnamed namespace

} // namespace untyped
} // namespace rma
} // namespace mgcom

