
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rpc.hpp>

#include "common/rma/rma.hpp"
#include "device/mpi/mpi_base.hpp"

#include <mgbase/logging/logger.hpp>

#include "contiguous.hpp"

#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {

namespace /*unnamed*/ {

class emulated_contiguous
{
    typedef emulated_contiguous self_type;
    
public:
    emulated_contiguous(rpc::requester& req, mpi_interface& mi)
        : tag_{1000}
        , mi_(mi)
        , req_(req)
        , comm_{ mi.comm_dup(MPI_COMM_WORLD, "MGCOM_COMM_RMA_EMULATOR") }
    {
        using rpc::register_handler;
        register_handler<am_read>(req, *this);
        register_handler<am_write>(req, *this);
    }
    
private:
    class am_read
    {
    public:
        static const mgcom::rpc::handler_id_t handler_id = 2100; // TODO: remove magic numbers
        
        struct argument_type {
            int         tag;
            const void* src_ptr;
            index_t     size_in_bytes;
        };
        typedef void    return_type;
        
        static return_type on_request(
            self_type&                              self
        ,   const mgcom::rpc::handler_parameters&   params
        ,   const argument_type&                    arg
        ) {
            ult::sync_flag flag;
            
            self.mi_.send_async(mgdev::mpi::send_async_params{
                mgdev::mpi::send_params{
                    arg.src_ptr
                ,   static_cast<int>(arg.size_in_bytes)
                ,   static_cast<int>(params.source)
                ,   arg.tag
                ,   self.get_comm()
                }
            ,   mgbase::make_callback_notify(&flag)
            });
            
            MGBASE_LOG_DEBUG(
                "msg:Started sending data for emulated get.\t"
                "src_proc:{}\t"
                "addr:{:x}\t"
                "size_in_bytes:{}\t"
                "tag:{}"
            ,   params.source
            ,   reinterpret_cast<mgbase::intptr_t>(arg.src_ptr)
            ,   arg.size_in_bytes
            ,   arg.tag
            );
            
            flag.wait();
            
            return;
        }
    };
    
public:
    ult::async_status<void> read_async(const rma::untyped::read_params& params)
    {
        const int tag = this->new_tag();
        
        // Wait for the local completion of MPI_Irecv().
        
        this->mi_.recv_async(mgdev::mpi::recv_async_params{
            mgdev::mpi::recv_params{
                rma::untyped::to_raw_pointer(params.dest_laddr)
            ,   static_cast<int>(params.size_in_bytes)
            ,   static_cast<int>(params.src_proc)
            ,   tag
            ,   this->get_comm()
            ,   MPI_STATUS_IGNORE
            }
        ,   params.on_complete
        });
        
        const am_read::argument_type arg{
            tag
        ,   rma::untyped::to_raw_pointer(params.src_raddr)
        ,   params.size_in_bytes
        };
        
        // The completion of RPC is ignored.
        
        while (! rpc::try_remote_call_async<am_read>(
            req_
        ,   params.src_proc
        ,   arg
        ,   mgbase::make_callback_empty()
        )) {
            ult::yield(); // will be removed
        }
        
        MGBASE_LOG_DEBUG(
            "msg:Started emulated get.\t"
            "src_proc:{}\t"
            "remote:{:x}\t"
            "local:{:x}\t"
            "size_in_bytes:{}\t"
            "tag:{}"
        ,   params.src_proc
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.src_raddr))
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.dest_laddr))
        ,   arg.size_in_bytes
        ,   arg.tag
        );
        
        return ult::make_async_deferred<void>();
    }
    
private:
    class am_write
    {
    public:
        static const mgcom::rpc::handler_id_t handler_id = 2102;
        
        struct argument_type {
            int         tag;
            void*       dest_ptr;
            index_t     size_in_bytes;
        };
        typedef void    return_type;
        
        static return_type on_request(
            self_type&                              self
        ,   const mgcom::rpc::handler_parameters&   params
        ,   const argument_type&                    arg
        ) {
            ult::sync_flag flag;
            
            self.mi_.recv_async(mgdev::mpi::recv_async_params{
                mgdev::mpi::recv_params{
                    arg.dest_ptr
                ,   static_cast<int>(arg.size_in_bytes)
                ,   static_cast<int>(params.source)
                ,   arg.tag
                ,   self.get_comm()
                ,   MPI_STATUS_IGNORE
                }
            ,   mgbase::make_callback_notify(&flag)
            });
            
            MGBASE_LOG_DEBUG(
                "msg:Start receiving data for emulated put."
                "\tsrc_proc:{}\taddr:{:x}\tsize_in_bytes:{}\ttag:{}"
            ,   params.source
            ,   reinterpret_cast<mgbase::intptr_t>(arg.dest_ptr)
            ,   arg.size_in_bytes
            ,   arg.tag
            );
            
            flag.wait();
            
            return;
        }
    };
    
public:
    ult::async_status<void> write_async(const rma::untyped::write_params& params)
    {
        const int tag = this->new_tag();
        
        this->mi_.send_async(mgdev::mpi::send_async_params{
            mgdev::mpi::send_params{
                rma::untyped::to_raw_pointer(params.src_laddr)
            ,   static_cast<int>(params.size_in_bytes)
            ,   static_cast<int>(params.dest_proc)
            ,   tag
            ,   this->get_comm()
            }
        ,   mgbase::make_callback_empty()
        });
        
        const am_write::argument_type arg{
            tag
        ,   rma::untyped::to_raw_pointer(params.dest_raddr)
        ,   params.size_in_bytes
        };
        
        // Wait for the completion of MPI_Irecv() at the destination node.
        
        while (! rpc::try_remote_call_async<am_write>(
            req_
        ,   params.dest_proc
        ,   arg
        ,   params.on_complete
        )) {
            ult::yield(); // will be removed
        }
        
        MGBASE_LOG_DEBUG(
            "msg:Started emulated put.\t"
            "dest_proc:{}\t"
            "remote:{:x}\t"
            "local:{:x}\t"
            "size_in_bytes:{}\ttag:{}"
        ,   params.dest_proc
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.dest_raddr))
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.src_laddr))
        ,   arg.size_in_bytes
        ,   arg.tag
        );
        
        return ult::make_async_deferred<void>();
    }
    
private:
    MPI_Comm get_comm()
    {
        return comm_;
    }
    
    int new_tag()
    {
        // TODO: limit of int (not a serious problem though)
        const int tag = tag_.fetch_add(1, mgbase::memory_order_relaxed);
        return tag;
    }
    
    mgbase::atomic<int> tag_;
    mpi_interface& mi_;
    rpc::requester& req_;
    MPI_Comm comm_;
};

} // unnamed namespace

} // namespace mpi
} // namespace mgcom

