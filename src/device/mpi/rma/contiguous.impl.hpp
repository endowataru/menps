
#pragma once

#include <mgcom/rma.hpp>
#include <mgcom/rpc/call2.hpp>

#include "common/rma/rma.hpp"
#include "device/mpi/mpi_base.hpp"

#include <mgbase/logging/logger.hpp>

#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {

template <typename Policy>
class emulated_contiguous
    : public virtual Policy::requester_interface_type
{
    typedef typename Policy::derived_type       derived_type;
    typedef typename Policy::handler_id_type    handler_id_type;

public:
    emulated_contiguous()
        : tag_{1000}
        , comm_{}
    { }
    
protected:
    void setup()
    {
        auto& self = this->derived();
        auto& rpc_rqstr = self.get_rpc_requester();
        auto& mi = self.get_mpi_interface();
        
        comm_ = mi.comm_dup(MPI_COMM_WORLD, "MGCOM_COMM_RMA_EMULATOR");
        
        Policy::register_handler(rpc_rqstr, read_handler{ &self });
        Policy::register_handler(rpc_rqstr, write_handler{ &self });
    }

private:
    struct read_handler
    {
        static const handler_id_type handler_id = 2100; // TODO: remove magic numbers
        
        derived_type* slf;
        
        struct request_type {
            int         tag;
            const void* src_ptr;
            index_t     size_in_bytes;
        };
        typedef void    reply_type;

        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& self = *slf;
            
            const auto& req = sc.request();
            const auto src_proc = sc.src_proc();

            MGBASE_LOG_DEBUG(
                "msg:Send data for emulated get.\t"
                "src_proc:{}\taddr:{:x}\tsize_in_bytes:{}\ttag:{}"
            ,   src_proc
            ,   reinterpret_cast<mgbase::intptr_t>(req.src_ptr)
            ,   req.size_in_bytes
            ,   req.tag
            );
            
            auto& mi = self.get_mpi_interface();
            
            mi.send(
                req.src_ptr
            ,   static_cast<int>(req.size_in_bytes)
            ,   static_cast<int>(src_proc)
            ,   req.tag
            ,   self.get_comm()
            );

            return sc.make_reply();
        }
    };

public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_read(const rma::untyped::read_params& params) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        
        const auto tag = this->new_tag();
        
        MGBASE_LOG_DEBUG(
            "msg:Execute emulated get.\t"
            "src_proc:{}\tremote:{:x}\tlocal:{:x}\tsize_in_bytes:{}\ttag:{}"
        ,   params.src_proc
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.src_raddr))
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.dest_laddr))
        ,   params.size_in_bytes
        ,   tag
        );
        
        // Execute MPI_Irecv().
        mi.recv_async(
            rma::untyped::to_raw_pointer(params.dest_laddr)
        ,   static_cast<int>(params.size_in_bytes)
        ,   static_cast<int>(params.src_proc)
        ,   tag
        ,   self.get_comm()
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        );
        
        const typename read_handler::request_type rqst{
            tag
        ,   rma::untyped::to_raw_pointer(params.src_raddr)
        ,   params.size_in_bytes
        };
        
        // The completion of RPC is ignored.
        // TODO: make this async
        Policy::template call<read_handler>(
            self.get_rpc_requester()
        ,   params.src_proc
        ,   rqst
        );
        
        return ult::make_async_deferred<void>();
    }

private:
    struct write_handler
    {
        static const handler_id_type handler_id = 2102;

        derived_type* slf;

        struct request_type {
            int         tag;
            void*       dest_ptr;
            index_t     size_in_bytes;
        };
        typedef void    reply_type;

        template <typename ServerCtx>
        typename ServerCtx::return_type operator() (ServerCtx& sc) const
        {
            auto& self = *slf;
            
            const auto& req = sc.request();
            const auto src_proc = sc.src_proc();

            MGBASE_LOG_DEBUG(
                "msg:Receive data for emulated put.\t"
                "src_proc:{}\taddr:{:x}\tsize_in_bytes:{}\ttag:{}"
            ,   src_proc
            ,   reinterpret_cast<mgbase::intptr_t>(req.dest_ptr)
            ,   req.size_in_bytes
            ,   req.tag
            );
            
            auto& mi = self.get_mpi_interface();
            
            mi.recv(
                req.dest_ptr
            ,   static_cast<int>(req.size_in_bytes)
            ,   static_cast<int>(src_proc)
            ,   req.tag
            ,   self.get_comm()
            ,   MPI_STATUS_IGNORE
            );

            return sc.make_reply();
        }
    };

public:
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_write(const rma::untyped::write_params& params) MGBASE_OVERRIDE
    {
        auto& self = this->derived();
        auto& mi = self.get_mpi_interface();
        
        const auto tag = this->new_tag();

        MGBASE_LOG_DEBUG(
            "msg:Execute emulated put.\t"
            "\tdest_proc:{}\tremote:{:x}\tlocal:{:x}\tsize_in_bytes:{}\ttag:{}"
        ,   params.dest_proc
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.dest_raddr))
        ,   static_cast<mgbase::intptr_t>(rma::to_integer(params.src_laddr))
        ,   params.size_in_bytes
        ,   tag
        );

        // Execute MPI_Isend().
        mi.send_async(
            rma::untyped::to_raw_pointer(params.src_laddr)
        ,   static_cast<int>(params.size_in_bytes)
        ,   static_cast<int>(params.dest_proc)
        ,   tag
        ,   this->get_comm()
        ,   mgbase::make_callback_empty()
        );

        const typename write_handler::request_type arg{
            tag
        ,   rma::untyped::to_raw_pointer(params.dest_raddr)
        ,   params.size_in_bytes
        };

        // Wait for the completion of MPI_Irecv() at the destination node.
        // TODO : make this async
        Policy::template call<write_handler>(
            self.get_rpc_requester()
        ,   params.dest_proc
        ,   arg
        //,   params.on_complete
        );
        
        return ult::make_async_ready(); // TODO: make this async
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
    MPI_Comm            comm_;
    
    derived_type& derived() MGBASE_NOEXCEPT {
        return static_cast<derived_type&>(*this);
    }
};

} // namespace mpi
} // namespace mgcom

