
#pragma once

#include "device/mpi/command/mpi_command.hpp"
#include "mpi3_completer.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "device/mpi3/mpi3_type.hpp"
#include "device/mpi3/rma/rma.hpp"
#include <mgbase/force_integer_cast.hpp>

namespace mgcom {
namespace mpi3 {

union mpi3_command_parameters
{
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
        mgbase::uint64_t    expected;
        mgbase::uint64_t    desired;
        void*               result_ptr;
        MPI_Datatype        datatype;
        int                 dest_rank;
        MPI_Aint            dest_index;
        mgbase::operation   on_complete;
    }
    compare_and_swap;
    
    struct fetch_and_op_parameters
    {
        mgbase::uint64_t    value;
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

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_get(
    const mpi3_command_parameters::get_parameters&  params
,   mpi3_completer&                                 completer
) {
    if (completer.full())
        return false;
    
    mpi3_error::check(
        MPI_Get(
            params.dest_ptr
        ,   params.size_in_bytes
        ,   MPI_BYTE
        ,   params.src_rank
        ,   params.src_index
        ,   params.size_in_bytes
        ,   MPI_BYTE
        ,   rma::get_win()
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Get.\t"
        "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tsize_in_bytes:{}"
    ,   params.src_rank
    ,   params.src_index
    ,   reinterpret_cast<mgbase::intptr_t>(params.dest_ptr)
    ,   params.size_in_bytes
    );
    
    completer.add_completion(params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_put(
    const mpi3_command_parameters::put_parameters&  params
,   mpi3_completer&                                 completer
) {
    if (completer.full())
        return false;
    
    mpi3_error::check(
        MPI_Put(
            params.src_ptr
        ,   params.size_in_bytes
        ,   MPI_BYTE
        ,   params.dest_rank
        ,   params.dest_index
        ,   params.size_in_bytes
        ,   MPI_BYTE
        ,   rma::get_win()
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Put.\t"
        "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tsize_in_bytes:{}"
    ,   reinterpret_cast<mgbase::intptr_t>(params.src_ptr)
    ,   params.dest_rank
    ,   params.dest_index
    ,   params.size_in_bytes
    );
    
    completer.add_completion(params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_compare_and_swap(
    const mpi3_command_parameters::compare_and_swap_parameters& params
,   mpi3_completer&                                             completer
) {
    if (completer.full())
        return false;
    
    /*
        TODO: const_cast is needed for OpenMPI 1.8.4.
    */
    mpi3_error::check(
        MPI_Compare_and_swap(
            const_cast<mgbase::uint64_t*>(&params.desired)      // origin_addr
        ,   const_cast<mgbase::uint64_t*>(&params.expected)     // compare_addr
        ,   params.result_ptr                       // result_addr
        ,   params.datatype                         // datatype
        ,   params.dest_rank                        // target_rank
        ,   params.dest_index                       // target_disp
        ,   rma::get_win()                          // win
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Compare_and_swap.\t"
        "desired:{}\texpected:{}\tresult_ptr:{:x}\t"
        "datatype:{}\tdest_rank:{}\tdest_index:{:x}"
    ,   params.desired
    ,   params.expected
    ,   reinterpret_cast<mgbase::intptr_t>(params.result_ptr)
    ,   get_datatype_name(params.datatype)
    ,   params.dest_rank
    ,   params.dest_index
    );
    
    completer.add_completion(params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_fetch_and_op(
    const mpi3_command_parameters::fetch_and_op_parameters& params
,   mpi3_completer&                                         completer
) {
    if (completer.full())
        return false;
    
    /*
        TODO: const_cast is needed for OpenMPI 1.8.4.
    */
    
    void* result_ptr = params.result_ptr;
    
    if (result_ptr == MGBASE_NULLPTR)
    {
        // Although MPI-3 requires the memory to store the results,
        // we don't need it in this case.
        static mgbase::uint64_t sink;
        result_ptr = &sink;
    }
    
    mpi3_error::check(
        MPI_Fetch_and_op(
            const_cast<mgbase::uint64_t*>(&params.value)    // origin_addr
        ,   result_ptr                          // result_addr
        ,   params.datatype                     // datatype
        ,   params.dest_rank                    // target_rank
        ,   params.dest_index                   // target_disp
        ,   params.operation                    // op
        ,   rma::get_win()                      // win
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Fetch_and_op.\t"
        "value:{}\tresult_ptr:{:x}\t"
        "dest_rank:{}\tdest_index:{:x}\tdatatype:{}\toperation:{}"
    ,   params.value
    ,   reinterpret_cast<mgbase::intptr_t>(params.result_ptr)
    ,   params.dest_rank
    ,   params.dest_index
    ,   get_datatype_name(params.datatype)
    ,   mgbase::force_integer_cast<mgbase::intptr_t>(params.operation)
    );
    
    completer.add_completion(params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_ibarrier(
    const mpi3_command_parameters::ibarrier_parameters&     params
,   mpi::mpi_completer&                                     completer
) {
    if (completer.full())
        return false;
    
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
    
    completer.complete(request, MPI_STATUS_IGNORE, params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_ibcast(
    const mpi3_command_parameters::ibcast_parameters&   params
,   mpi::mpi_completer&                                 completer
) {
    if (completer.full())
        return false;
    
    MPI_Request request;
    
    mpi_error::check(
        MPI_Ibcast(
            params.ptr
        ,   static_cast<int>(params.number_of_bytes)
        ,   MPI_BYTE
        ,   static_cast<int>(params.root)
        ,   params.comm
        ,   &request
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Ibcast.\t"
        "root:{}\tptr:\tsize_in_bytes:{}"
    ,   params.root
    ,   reinterpret_cast<mgbase::intptr_t>(params.ptr)
    ,   params.number_of_bytes
    );
    
    completer.complete(request, MPI_STATUS_IGNORE, params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_iallgather(
    const mpi3_command_parameters::iallgather_parameters&   params
,   mpi::mpi_completer&                                     completer
) {
    if (completer.full())
        return false;
    
    MPI_Request request;
    
    mpi_error::check(
        MPI_Iallgather(
            params.src
        ,   static_cast<int>(params.number_of_bytes)
        ,   MPI_BYTE
        ,   params.dest
        ,   static_cast<int>(params.number_of_bytes)
        ,   MPI_BYTE
        ,   params.comm
        ,   &request
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Iallgather.\t"
        "src:{}\tdest:\tsize_in_bytes:{}"
    ,   reinterpret_cast<mgbase::intptr_t>(params.src)
    ,   reinterpret_cast<mgbase::intptr_t>(params.dest)
    ,   params.number_of_bytes
    );
    
    completer.complete(request, MPI_STATUS_IGNORE, params.on_complete);
    
    return true;
}

MGBASE_ALWAYS_INLINE MGBASE_WARN_UNUSED_RESULT
bool try_execute_ialltoall(
    const mpi3_command_parameters::ialltoall_parameters&    params
,   mpi::mpi_completer&                                     completer
) {
    if (completer.full())
        return false;
    
    MPI_Request request;
    
    mpi_error::check(
        MPI_Iallgather(
            params.src
        ,   static_cast<int>(params.number_of_bytes)
        ,   MPI_BYTE
        ,   params.dest
        ,   static_cast<int>(params.number_of_bytes)
        ,   MPI_BYTE
        ,   params.comm
        ,   &request
        )
    );
    
    MGBASE_LOG_DEBUG(
        "msg:Executed MPI_Ialltoall.\t"
        "src:{}\tdest:\tsize_in_bytes:{}"
    ,   reinterpret_cast<mgbase::intptr_t>(params.src)
    ,   reinterpret_cast<mgbase::intptr_t>(params.dest)
    ,   params.number_of_bytes
    );
    
    completer.complete(request, MPI_STATUS_IGNORE, params.on_complete);
    
    return true;
}

#define MGCOM_MPI3_COMMAND_CODES(x)         \
        x(MPI3_COMMAND_GET)                 \
    ,   x(MPI3_COMMAND_PUT)                 \
    ,   x(MPI3_COMMAND_COMPARE_AND_SWAP)    \
    ,   x(MPI3_COMMAND_FETCH_AND_OP)        \
    ,   x(MPI3_COMMAND_IBARRIER)            \
    ,   x(MPI3_COMMAND_IBCAST)              \
    ,   x(MPI3_COMMAND_IALLGATHER)          \
    ,   x(MPI3_COMMAND_IALLTOALL)            

#define MGCOM_MPI3_COMMAND_EXECUTE_CASES(CASE, RETURN, params, completer) \
    CASE(MPI3_COMMAND_GET): { \
        const bool ret = ::mgcom::mpi3::try_execute_get((params).get, (completer)); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_PUT): { \
        const bool ret = ::mgcom::mpi3::try_execute_put((params).put, (completer)); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_COMPARE_AND_SWAP): { \
        const bool ret = ::mgcom::mpi3::try_execute_compare_and_swap((params).compare_and_swap, (completer)); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_FETCH_AND_OP): { \
        const bool ret = ::mgcom::mpi3::try_execute_fetch_and_op((params).fetch_and_op, (completer)); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_IBARRIER): { \
        const bool ret = ::mgcom::mpi3::try_execute_ibarrier((params).ibarrier, (completer).get_mpi1_completer()); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_IBCAST): { \
        const bool ret = ::mgcom::mpi3::try_execute_ibcast((params).ibcast, (completer).get_mpi1_completer()); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_IALLGATHER): { \
        const bool ret = ::mgcom::mpi3::try_execute_iallgather((params).iallgather, (completer).get_mpi1_completer()); \
        RETURN(ret); \
    } \
    CASE(MPI3_COMMAND_IALLTOALL): { \
        const bool ret = ::mgcom::mpi3::try_execute_ialltoall((params).ialltoall, (completer).get_mpi1_completer()); \
        RETURN(ret); \
    }

} // namespace mpi3
} // namespace mgcom

