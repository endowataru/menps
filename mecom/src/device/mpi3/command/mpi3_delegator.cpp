
#include "mpi3_delegator.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "common/command/delegate.hpp"
#include "device/mpi3/mpi3_type.hpp"
#include "device/mpi/mpi_base.hpp"
#include <menps/mefdn/force_integer_cast.hpp>

namespace menps {
namespace mecom {
namespace mpi3 {

namespace /*unnamed*/ {

// point-to-point communication

struct get_async_closure
{
    get_async_params            params;
    mpi::mpi_completer_base*    comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi3_error::check(
            MPI_Rget(
                pb.dest_ptr     // origin_addr
            ,   pb.num_bytes    // origin_count
            ,   MPI_BYTE        // origin_datatype
            ,   pb.src_rank     // target_rank
            ,   pb.src_index    // target_disp
            ,   pb.num_bytes    // target_count
            ,   MPI_BYTE        // target_datatype
            ,   pb.win          // win
            ,   &request        // request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Rget.\t"
            "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tnum_bytes:{}"
        ,   pb.src_rank
        ,   pb.src_index
        ,   reinterpret_cast<mefdn::intptr_t>(pb.dest_ptr)
        ,   pb.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct put_async_closure
{
    put_async_params            params;
    mpi::mpi_completer_base*    comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi3_error::check(
            MPI_Rput(
                pb.src_ptr      // origin_addr
            ,   pb.num_bytes    // origin_count
            ,   MPI_BYTE        // origin_datatype
            ,   pb.dest_rank    // target_rank
            ,   pb.dest_index   // target_disp
            ,   pb.num_bytes    // target_count
            ,   MPI_BYTE        // target_datatype
            ,   pb.win          // win
            ,   &request        // request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Rput.\t"
            "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tnum_bytes:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.src_ptr)
        ,   pb.dest_rank
        ,   pb.dest_index
        ,   pb.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        // TODO: To complete RMA, we need to flush this.
        //       This operation is very expensive.
        MPI_Win_flush(pb.dest_rank, pb.win);
        
        return true;
    }
};

// atomic operations

struct compare_and_swap_closure
{
    compare_and_swap_async_params   params;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        const auto& pb = params.base;
        
        /*
            TODO: const_cast is needed for OpenMPI 1.8.4.
        */
        mpi3_error::check(
            MPI_Compare_and_swap(
                const_cast<mefdn::uint64_t*>(&pb.desired)  // origin_addr
            ,   const_cast<mefdn::uint64_t*>(&pb.expected) // compare_addr
            ,   pb.result_ptr                               // result_addr
            ,   pb.datatype                                 // datatype
            ,   pb.dest_rank                                // target_rank
            ,   pb.dest_index                               // target_disp
            ,   pb.win                                      // win
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Compare_and_swap.\t"
            "desired:{}\texpected:{}\tresult_ptr:{:x}\t"
            "datatype:{}\tdest_rank:{}\tdest_index:{:x}"
        ,   pb.desired
        ,   pb.expected
        ,   reinterpret_cast<mefdn::intptr_t>(pb.result_ptr)
        ,   get_datatype_name(pb.datatype)
        ,   pb.dest_rank
        ,   pb.dest_index
        );
        
        // TODO: To complete atomics, we need to flush this.
        //       This operation is very expensive.
        MPI_Win_flush_all(pb.win);
        
        // Execute the callback.
        params.on_complete();
        
        return true;
    }
};

struct fetch_and_op_closure
{
    fetch_and_op_async_params   params;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        /*
            TODO: const_cast is needed for OpenMPI 1.8.4.
        */
        
        const auto& pb = params.base;
        
        void* result_ptr = pb.result_ptr;
        
        if (result_ptr == nullptr)
        {
            // Although MPI-3 requires the memory to store the results,
            // we don't need it in this case.
            static mefdn::uint64_t sink;
            result_ptr = &sink;
        }
        
        mpi3_error::check(
            MPI_Fetch_and_op(
                const_cast<mefdn::uint64_t*>(&pb.value)    // origin_addr
            ,   result_ptr                          // result_addr
            ,   pb.datatype                     // datatype
            ,   pb.dest_rank                    // target_rank
            ,   pb.dest_index                   // target_disp
            ,   pb.operation                    // op
            ,   pb.win                          // win
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Fetch_and_op.\t"
            "value:{}\tresult_ptr:{:x}\t"
            "dest_rank:{}\tdest_index:{:x}\tdatatype:{}\toperation:{}"
        ,   pb.value
        ,   reinterpret_cast<mefdn::intptr_t>(pb.result_ptr)
        ,   pb.dest_rank
        ,   pb.dest_index
        ,   get_datatype_name(pb.datatype)
        ,   mefdn::force_integer_cast<mefdn::intptr_t>(pb.operation)
        );
        
        // TODO: To complete atomics, we need to flush this.
        //       This operation is very expensive.
        MPI_Win_flush_all(pb.win);
        
        // Execute the callback.
        params.on_complete();
        
        return true;
    }
};

// registration

struct attach_closure
{
    attach_params   params;
    MPI_Aint*       addr_result;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        const auto& p = params;
        
        MEFDN_ASSERT(p.ptr != nullptr);
        MEFDN_ASSERT(p.size > 0);
        
        MEFDN_LOG_DEBUG(
            "msg:Attach a memory region.\tptr:{:x}\tsize:{}"
        ,   reinterpret_cast<mefdn::uint64_t>(p.ptr)
        ,   p.size
        );
        
        mpi3_error::check(
            MPI_Win_attach(p.win, p.ptr, p.size)
        );
        
        mpi3_error::check(
            MPI_Get_address(p.ptr, this->addr_result)
        );
        
        return true;
    }
};

struct detach_closure
{
    detach_params   params;
    
    MEFDN_NODISCARD
    bool operator () () const
    {
        MEFDN_ASSERT(params.ptr != nullptr);
        
        MEFDN_LOG_DEBUG(
            "msg:Detach a memory region.\tptr:{:x}"
        ,   reinterpret_cast<mefdn::uint64_t>(params.ptr)
        );
        
        mpi3_error::check(
            MPI_Win_detach(params.win, params.ptr)
        );
        
        return true;
    }
};

// collective operations

struct barrier_async_closure
{
    barrier_async_params        params;
    mpi::mpi_completer_base*    comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Ibarrier(
                pb.comm // TODO
            ,   &request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Ibarrier."
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct broadcast_async_closure
{
    broadcast_async_params      params;
    mpi::mpi_completer_base*    comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Ibcast(
                pb.ptr
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   static_cast<int>(pb.root)
            ,   pb.comm
            ,   &request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Ibcast.\t"
            "root:{}\tptr:{:x}\tnum_bytes:{}"
        ,   pb.root
        ,   reinterpret_cast<mefdn::intptr_t>(pb.ptr)
        ,   pb.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct allgather_async_closure
{
    allgather_async_params      params;
    mpi::mpi_completer_base*    comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Iallgather(
                pb.src
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.dest
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.comm
            ,   &request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Iallgather.\t"
            "src:{}\tdest:\tnum_bytes:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.src)
        ,   reinterpret_cast<mefdn::intptr_t>(pb.dest)
        ,   pb.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct alltoall_async_closure
{
    alltoall_async_params       params;
    mpi::mpi_completer_base*    comp;
    
    MEFDN_NODISCARD
    bool operator() () const
    {
        if (MEFDN_UNLIKELY(comp->full()))
            return false;
        
        const auto& pb = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Ialltoall(
                pb.src
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.dest
            ,   static_cast<int>(pb.num_bytes)
            ,   MPI_BYTE
            ,   pb.comm
            ,   &request
            )
        );
        
        MEFDN_LOG_DEBUG(
            "msg:Executed MPI_Ialltoall.\t"
            "src:{}\tdest:\tnum_bytes:{}"
        ,   reinterpret_cast<mefdn::intptr_t>(pb.src)
        ,   reinterpret_cast<mefdn::intptr_t>(pb.dest)
        ,   pb.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

} // unnamed namespace

// point-to-point communication

ult::async_status<void> mpi3_delegator::get_async(const get_async_params params)
{
    const auto pb = params.base;
    
    MEFDN_ASSERT(pb.dest_ptr != nullptr);
    //MEFDN_ASSERT(mpi::is_valid_rank(this->get_endpoint(), pb.src_rank)); // TODO
    MEFDN_ASSERT(pb.num_bytes > 0);
    
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MEFDN_ASSERT(pb.src_index != 0);
    
    delegate(
        this->get_delegator()
    ,   get_async_closure{ params, &this->get_completer() }
    );
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Rget.\t"
        "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tnum_bytes:{}"
    ,   pb.src_rank
    ,   pb.src_index
    ,   reinterpret_cast<mefdn::intptr_t>(pb.dest_ptr)
    ,   pb.num_bytes
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi3_delegator::put_async(const put_async_params params)
{
    const auto pb = params.base;
    
    MEFDN_ASSERT(pb.src_ptr != nullptr);
    //MEFDN_ASSERT(mpi::is_valid_rank(this->get_endpoint(), pb.dest_rank)); // TODO
    MEFDN_ASSERT(pb.num_bytes > 0);
    
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MEFDN_ASSERT(pb.dest_index != 0);
    
    delegate(
        this->get_delegator()
    ,   put_async_closure{ params, &this->get_completer() }
    );
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Rput.\t"
        "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tnum_bytes:{}"
    ,   reinterpret_cast<mefdn::intptr_t>(pb.src_ptr)
    ,   pb.dest_rank
    ,   pb.dest_index
    ,   pb.num_bytes
    );
    
    return ult::make_async_deferred<void>();
}

// atomic operations

ult::async_status<void> mpi3_delegator::compare_and_swap_async(const compare_and_swap_async_params params)
{
    const auto pb = params.base;
    
    //MEFDN_ASSERT(mpi::is_valid_rank(this->get_endpoint(), params.dest_rank)); // TODO
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MEFDN_ASSERT(pb.dest_index != 0);
    
    delegate(
        this->get_delegator()
    ,   compare_and_swap_closure{ params }
    );
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Compare_and_swap.\t"
        "desired:{}\texpected:{}\tresult_ptr:{:x}\t"
        "datatype:{}\tdest_rank:{}\tdest_index:{:x}"
    ,   pb.desired
    ,   pb.expected
    ,   reinterpret_cast<mefdn::intptr_t>(pb.result_ptr)
    ,   get_datatype_name(pb.datatype)
    ,   pb.dest_rank
    ,   pb.dest_index
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi3_delegator::fetch_and_op_async(const fetch_and_op_async_params params)
{
    const auto pb = params.base;
    
    //MEFDN_ASSERT(mpi::is_valid_rank(this->get_endpoint(), pb.dest_rank));
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MEFDN_ASSERT(pb.dest_index != 0);
       
    try_delegate(
        this->get_delegator()
    ,   fetch_and_op_closure{ params }
    );
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Fetch_and_op.\t"
        "value:{}\tresult_ptr:{:x}\t"
        "dest_rank:{}\tdest_index:{:x}\tdatatype:{}\toperation:{}"
    ,   pb.value
    ,   reinterpret_cast<mefdn::intptr_t>(pb.result_ptr)
    ,   pb.dest_rank
    ,   pb.dest_index
    ,   get_datatype_name(pb.datatype)
    ,   mefdn::force_integer_cast<mefdn::intptr_t>(pb.operation)
    );
    
    return ult::make_async_deferred<void>();
}

// registration

MPI_Aint mpi3_delegator::attach(const attach_params params)
{
    MPI_Aint ret{};
    
    execute(
        this->get_delegator()
    ,   attach_closure{ params, &ret }
    );
    
    return ret;
}

void mpi3_delegator::detach(const detach_params params)
{
    execute(
        this->get_delegator()
    ,   detach_closure{ params }
    );
}

// collective operations

ult::async_status<void> mpi3_delegator::barrier_async(const barrier_async_params params)
{
    delegate(
        this->get_delegator()
    ,   barrier_async_closure{ params, &this->get_completer() }
    );
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Ibarrier."
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi3_delegator::broadcast_async(const broadcast_async_params params)
{
    delegate(
        this->get_delegator()
    ,   broadcast_async_closure{ params, &this->get_completer() }
    );
    
    const auto pb = params.base;
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Ibcast.\t"
        "root:{}\tptr:\tnum_bytes:{}"
    ,   pb.root
    ,   reinterpret_cast<mefdn::intptr_t>(pb.ptr)
    ,   pb.num_bytes
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi3_delegator::allgather_async(const allgather_async_params params)
{
    delegate(
        this->get_delegator()
    ,   allgather_async_closure{ params, &this->get_completer() }
    );
    
    const auto pb = params.base;
    
    MEFDN_LOG_DEBUG(
        "msg:Delegated MPI_Iallgather.\t"
        "src:{}\tdest:\tnum_bytes:{}"
    ,   reinterpret_cast<mefdn::intptr_t>(pb.src)
    ,   reinterpret_cast<mefdn::intptr_t>(pb.dest)
    ,   pb.num_bytes
    );
    
    return ult::make_async_deferred<void>();
}

ult::async_status<void> mpi3_delegator::alltoall_async(const alltoall_async_params params)
{
    delegate(
        this->get_delegator()
    ,   alltoall_async_closure{ params, &this->get_completer() }
    );
    
    const auto pb = params.base;
    
    MEFDN_LOG_DEBUG(
        "msg:Queued MPI_Ialltoall.\t"
        "root:{}\tptr:\tnum_bytes:{}"
    ,   reinterpret_cast<mefdn::intptr_t>(pb.src)
    ,   reinterpret_cast<mefdn::intptr_t>(pb.dest)
    ,   pb.num_bytes
    );
    
    return ult::make_async_deferred<void>();
}

} // namespace mpi3
} // namespace mecom
} // namespace menps

