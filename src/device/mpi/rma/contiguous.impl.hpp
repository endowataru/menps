
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
namespace rma {

namespace /*unnamed*/ {

class emulated_contiguous
{
public:
    emulated_contiguous()
    {
        tag_.store(1000, mgbase::memory_order_relaxed);
    }
    
    template <emulated_contiguous& self>
    static void initialize(mpi_interface& mi)
    {
        self.mi_ = &mi;
        
        mgcom::rpc::register_handler< am_read<self> >();
        mgcom::rpc::register_handler< am_write<self> >();
        
        self.comm_ = mi.comm_dup(MPI_COMM_WORLD, "MGCOM_COMM_RMA_EMULATOR");
    }
    
    void finalize()
    {
        // do nothing
    }
    
private:
    template <emulated_contiguous& self>
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
            const mgcom::rpc::handler_parameters&   params
        ,   const argument_type&                    arg
        ) {
            mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
            
            self.mi_->isend({
                arg.src_ptr
            ,   static_cast<int>(arg.size_in_bytes)
            ,   static_cast<int>(params.source)
            ,   arg.tag
            ,   self.get_comm()
            ,   mgbase::make_operation_store_release(&finished, true)
            });
            
            MGBASE_LOG_DEBUG("msg:Started sending data for emulated get."
                "\tsrc_proc:{}\taddr:{:x}\tsize_in_bytes:{}\ttag:{}"
            ,   params.source
            ,   reinterpret_cast<mgbase::intptr_t>(arg.src_ptr)
            ,   arg.size_in_bytes
            ,   arg.tag
            );
            
            // TODO: busy loop
            while (!finished.load(mgbase::memory_order_acquire)) { }
            
            return;
        }
    };
    
public:
    template <emulated_contiguous& self>
    static bool try_read(const untyped::read_params& params)
    {
        const int tag = self.new_tag();
        
        const typename am_read<self>::argument_type arg = {
            tag
        ,   untyped::to_raw_pointer(params.src_raddr)
        ,   params.size_in_bytes
        };
        
        // The completion of RPC is ignored.
        
        const bool ret = rpc::try_remote_call_async< am_read<self> >(
            params.src_proc
        ,   arg
        ,   mgbase::make_no_operation()
        );
        
        if (ret)
        {
            // Wait for the local completion of MPI_Irecv().
            
            self.mi_->irecv({
                untyped::to_raw_pointer(params.dest_laddr)
            ,   static_cast<int>(params.size_in_bytes)
            ,   static_cast<int>(params.src_proc)
            ,   tag
            ,   self.get_comm()
            ,   MPI_STATUS_IGNORE
            ,   params.on_complete
            });
        }
        
        MGBASE_LOG_DEBUG("msg:{}"
            "\tsrc_proc:{}\tremote:{:x}\tlocal:{:x}\tsize_in_bytes:{}\ttag:{}"
        ,   (ret ? "Started emulated get." : "Failed to start emulated get.")
        ,   params.src_proc
        ,   static_cast<mgbase::intptr_t>(to_integer(params.src_raddr))
        ,   static_cast<mgbase::intptr_t>(to_integer(params.dest_laddr))
        ,   arg.size_in_bytes
        ,   arg.tag
        );
        
        return ret;
    }
    
private:
    template <emulated_contiguous& self>
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
            const mgcom::rpc::handler_parameters& params
        ,   const argument_type& arg
        ) {
            mgbase::atomic<bool> finished = MGBASE_ATOMIC_VAR_INIT(false);
            
            self.mi_->irecv({
                arg.dest_ptr
            ,   static_cast<int>(arg.size_in_bytes)
            ,   static_cast<int>(params.source)
            ,   arg.tag
            ,   self.get_comm()
            ,   MPI_STATUS_IGNORE
            ,   mgbase::make_operation_store_release(&finished, true)
            });
            
            MGBASE_LOG_DEBUG("msg:Start receiving data for emulated put."
                "\tsrc_proc:{}\taddr:{:x}\tsize_in_bytes:{}\ttag:{}"
            ,   params.source
            ,   reinterpret_cast<mgbase::intptr_t>(arg.dest_ptr)
            ,   arg.size_in_bytes
            ,   arg.tag
            );
            
            // TODO: busy loop
            while (!finished.load(mgbase::memory_order_acquire)) { }
            
            return;
        }
    };
    
public:
    template <emulated_contiguous& self>
    static bool try_write(const untyped::write_params& params)
    {
        const int tag = self.new_tag();
        
        const typename am_write<self>::argument_type arg = {
            tag
        ,   untyped::to_raw_pointer(params.dest_raddr)
        ,   params.size_in_bytes
        };
        
        // Wait for the completion of MPI_Irecv() at the destination node.
        
        const bool ret = rpc::try_remote_call_async< am_write<self> >(
            params.dest_proc
        ,   arg
        ,   params.on_complete
        );
        
        if (MGBASE_LIKELY(ret))
        {
            self.mi_->isend({
                untyped::to_raw_pointer(params.src_laddr)
            ,   static_cast<int>(params.size_in_bytes)
            ,   static_cast<int>(params.dest_proc)
            ,   tag
            ,   self.get_comm()
            ,   mgbase::make_no_operation()
            });
        }
        
        MGBASE_LOG_DEBUG("msg:{}"
            "\tdest_proc:{}\tremote:{:x}\tlocal:{:x}\tsize_in_bytes:{}\ttag:{}"
        ,   (ret ? "Started emulated put." : "Failed to start emulated put.")
        ,   params.dest_proc
        ,   static_cast<mgbase::intptr_t>(to_integer(params.dest_raddr))
        ,   static_cast<mgbase::intptr_t>(to_integer(params.src_laddr))
        ,   arg.size_in_bytes
        ,   arg.tag
        );
        
        return ret;
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
    
    MPI_Comm comm_;
    mgbase::atomic<int> tag_;
    mpi_interface* mi_;
};

} // unnamed namespace

} // namespace rma
} // namespace mpi
} // namespace mgcom

