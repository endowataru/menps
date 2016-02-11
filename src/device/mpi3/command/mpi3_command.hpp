
#pragma once

#include "device/mpi/command/mpi_command.hpp"
#include "mpi3_completer.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "device/mpi3/rma.hpp"

namespace mgcom {
namespace mpi3 {

enum mpi3_command_code
{
    MPI3_COMMAND_GET = mpi::MPI_COMMAND_END + 1
,   MPI3_COMMAND_PUT
,   MPI3_COMMAND_COMPARE_AND_SWAP
,   MPI3_COMMAND_FETCH_AND_OP
,   MPI3_COMMAND_IBARRIER
,   MPI3_COMMAND_IBCAST
,   MPI3_COMMAND_IALLGATHER
,   MPI3_COMMAND_IALLTOALL
,   MPI3_COMMAND_END
};


namespace detail {

inline std::string get_datatype_name(const MPI_Datatype datatype) {
    char buf[MPI_MAX_OBJECT_NAME];
    int len;
    
    mpi3_error::check(
        MPI_Type_get_name(datatype, buf, &len)
    );
    
    return buf;
}

} // namespace detail

union mpi3_command_parameters
{
    mpi::mpi_command_parameters mpi1;
    
    struct get_parameters
    {
        void*               dest_ptr;
        int                 src_rank;
        MPI_Aint            src_index;
        int                 size_in_bytes;
        mgbase::operation   on_complete;
    }
    get;
    
    struct put_parameters
    {
        const void*         src_ptr;
        int                 dest_rank;
        MPI_Aint            dest_index;
        int                 size_in_bytes;
        mgbase::operation   on_complete;
    }
    put;
    
    struct compare_and_swap_parameters
    {
        const void*         expected_ptr;
        const void*         desired_ptr;
        void*               result_ptr;
        MPI_Datatype        datatype;
        int                 dest_rank;
        MPI_Aint            dest_index;
        mgbase::operation   on_complete;
    }
    compare_and_swap;
    
    struct fetch_and_op_parameters
    {
        const void*         value_ptr;
        void*               result_ptr;
        MPI_Datatype        datatype;
        int                 dest_rank;
        MPI_Aint            dest_index;
        MPI_Op              operation;
        mgbase::operation   on_complete;
    }
    fetch_and_op;
    
    struct ibarrier_parameters
    {
        MPI_Comm            comm;
        mgbase::operation   on_complete;
    }
    ibarrier;
    
    struct ibcast_parameters
    {
        process_id_t        root;
        void*               ptr;
        index_t             number_of_bytes;
        MPI_Comm            comm;
        mgbase::operation   on_complete;
    }
    ibcast;
    
    struct iallgather_parameters
    {
        const void*         src;
        void*               dest;
        index_t             number_of_bytes;
        MPI_Comm            comm;
        mgbase::operation   on_complete;
    }
    iallgather;
    
    struct ialltoall_parameters
    {
        const void*         src;
        void*               dest;
        index_t             number_of_bytes;
        MPI_Comm            comm;
        mgbase::operation   on_complete;
    }
    ialltoall;
};

MGBASE_ALWAYS_INLINE bool execute_on_this_thread(
    const mpi3_command_code         code
,   const mpi3_command_parameters&  params
,   mpi3_completer&                 completer
)
{
    if (code < static_cast<mpi3_command_code>(mpi::MPI_COMMAND_END)) {
        return mpi::execute_on_this_thread(
            static_cast<mpi::mpi_command_code>(code)
        ,   params.mpi1
        ,   completer.get_mpi1_completer()
        );
    }
    
    MGBASE_ASSERT(MPI3_COMMAND_GET <= code && code < MPI3_COMMAND_END);
    
    switch (code) {
        case MPI3_COMMAND_GET: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::get_parameters& p = params.get;
            
            mpi3_error::check(
                MPI_Get(
                    p.dest_ptr
                ,   p.size_in_bytes
                ,   MPI_BYTE
                ,   p.src_rank
                ,   p.src_index
                ,   p.size_in_bytes
                ,   MPI_BYTE
                ,   rma::get_win()
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Get.\t"
                "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tsize_in_bytes:{}"
            ,   p.src_rank
            ,   p.src_index
            ,   reinterpret_cast<mgbase::intptr_t>(p.dest_ptr)
            ,   p.size_in_bytes
            );
            
            completer.add_completion(p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_PUT:{
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::put_parameters& p = params.put;
            
            mpi3_error::check(
                MPI_Put(
                    p.src_ptr
                ,   p.size_in_bytes
                ,   MPI_BYTE
                ,   p.dest_rank
                ,   p.dest_index
                ,   p.size_in_bytes
                ,   MPI_BYTE
                ,   rma::get_win()
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Put.\t"
                "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tsize_in_bytes:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.src_ptr)
            ,   p.dest_rank
            ,   p.dest_index
            ,   p.size_in_bytes
            );
            
            completer.add_completion(p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_COMPARE_AND_SWAP: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::compare_and_swap_parameters& p = params.compare_and_swap;
            
            /*
                TODO: const_cast is needed for OpenMPI 1.8.4.
            */
            mpi3_error::check(
                MPI_Compare_and_swap(
                    const_cast<void*>(p.desired_ptr)    // origin_addr
                ,   const_cast<void*>(p.expected_ptr)   // compare_addr
                ,   p.result_ptr                        // result_addr
                ,   p.datatype                          // datatype
                ,   p.dest_rank                         // target_rank
                ,   p.dest_index                        // target_disp
                ,   rma::get_win()                      // win
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Compare_and_swap.\t"
                "desired_ptr:{:x}\texpected_ptr:{:x}\tresult_ptr:{:x}\t"
                "datatype:{}\tdest_rank:{}\tdest_index:{:x}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.desired_ptr)
            ,   reinterpret_cast<mgbase::intptr_t>(p.expected_ptr)
            ,   reinterpret_cast<mgbase::intptr_t>(p.result_ptr)
            ,   detail::get_datatype_name(p.datatype)
            ,   p.dest_rank
            ,   p.dest_index
            );
            
            completer.add_completion(p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_FETCH_AND_OP: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::fetch_and_op_parameters& p = params.fetch_and_op;
            
            /*
                TODO: const_cast is needed for OpenMPI 1.8.4.
            */
            
            mpi3_error::check(
                MPI_Fetch_and_op(
                    const_cast<void*>(p.value_ptr)  // origin_addr
                ,   p.result_ptr                    // result_addr
                ,   p.datatype                      // datatype
                ,   p.dest_rank                     // target_rank
                ,   p.dest_index                    // target_disp
                ,   p.operation                     // op
                ,   rma::get_win()                  // win
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Fetch_and_op.\t"
                "value_ptr:{:x}\tresult_ptr:{:x}\t"
                "dest_rank:{}\tdest_index:{:x}\tdatatype:{}\toperation:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.value_ptr)
            ,   reinterpret_cast<mgbase::intptr_t>(p.result_ptr)
            ,   p.dest_rank
            ,   p.dest_index
            ,   detail::get_datatype_name(p.datatype)
            ,   reinterpret_cast<mgbase::intptr_t>(p.operation)
            );
            
            completer.add_completion(p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_IBARRIER: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::ibarrier_parameters& p = params.ibarrier;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Ibarrier(
                    MPI_COMM_WORLD // TODO
                ,   &request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Ibarrier."
            );
            
            completer.get_mpi1_completer()
                .complete(request, MPI_STATUS_IGNORE, p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_IBCAST: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::ibcast_parameters& p = params.ibcast;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Ibcast(
                    p.ptr
                ,   static_cast<int>(p.number_of_bytes)
                ,   MPI_BYTE
                ,   static_cast<int>(p.root)
                ,   p.comm
                ,   &request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Ibcast.\t"
                "root:{}\tptr:\tsize_in_bytes:{}"
            ,   p.root
            ,   reinterpret_cast<mgbase::intptr_t>(p.ptr)
            ,   p.number_of_bytes
            );
            
            completer.get_mpi1_completer()
                .complete(request, MPI_STATUS_IGNORE, p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_IALLGATHER: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::iallgather_parameters& p = params.iallgather;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Iallgather(
                    p.src
                ,   static_cast<int>(p.number_of_bytes)
                ,   MPI_BYTE
                ,   p.dest
                ,   static_cast<int>(p.number_of_bytes)
                ,   MPI_BYTE
                ,   p.comm
                ,   &request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Iallgather.\t"
                "src:{}\tdest:\tsize_in_bytes:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.src)
            ,   reinterpret_cast<mgbase::intptr_t>(p.dest)
            ,   p.number_of_bytes
            );
            
            completer.get_mpi1_completer()
                .complete(request, MPI_STATUS_IGNORE, p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_IALLTOALL: {
            if (completer.full())
                return false;
            
            const mpi3_command_parameters::ialltoall_parameters& p = params.ialltoall;
            
            MPI_Request request;
            
            mpi_error::check(
                MPI_Iallgather(
                    p.src
                ,   static_cast<int>(p.number_of_bytes)
                ,   MPI_BYTE
                ,   p.dest
                ,   static_cast<int>(p.number_of_bytes)
                ,   MPI_BYTE
                ,   p.comm
                ,   &request
                )
            );
            
            MGBASE_LOG_DEBUG(
                "msg:Executed MPI_Ialltoall.\t"
                "src:{}\tdest:\tsize_in_bytes:{}"
            ,   reinterpret_cast<mgbase::intptr_t>(p.src)
            ,   reinterpret_cast<mgbase::intptr_t>(p.dest)
            ,   p.number_of_bytes
            );
            
            completer.get_mpi1_completer()
                .complete(request, MPI_STATUS_IGNORE, p.on_complete);
            
            return true;
        }
        
        case MPI3_COMMAND_END:
            MGBASE_UNREACHABLE();
    }
}

} // namespace mpi3
} // namespace mgcom

