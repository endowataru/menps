
#include "mpi3_delegator.hpp"
#include "device/mpi3/mpi3_error.hpp"
#include "common/command/delegate.hpp"
#include "device/mpi3/mpi3_type.hpp"
#include "device/mpi/mpi_base.hpp"

namespace mgcom {
namespace mpi3 {

namespace /*unnamed*/ {

// point-to-point communication

struct rget_closure
{
    rget_params                 params;
    mpi::mpi_completer_base*    comp;
    MPI_Win                     win;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        MPI_Request request;
        
        mpi3_error::check(
            MPI_Rget(
                params.dest_ptr         // origin_addr
            ,   params.size_in_bytes    // origin_count
            ,   MPI_BYTE                // origin_datatype
            ,   params.src_rank         // target_rank
            ,   params.src_index        // target_disp
            ,   params.size_in_bytes    // target_count
            ,   MPI_BYTE                // target_datatype
            ,   win                     // win
            ,   &request                // request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Rget.\t"
            "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tsize_in_bytes:{}"
        ,   params.src_rank
        ,   params.src_index
        ,   reinterpret_cast<mgbase::intptr_t>(params.dest_ptr)
        ,   params.size_in_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct rput_closure
{
    rput_params                 params;
    mpi::mpi_completer_base*    comp;
    MPI_Win                     win;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        MPI_Request request;
        
        mpi3_error::check(
            MPI_Rput(
                params.src_ptr          // origin_addr
            ,   params.size_in_bytes    // origin_count
            ,   MPI_BYTE                // origin_datatype
            ,   params.dest_rank        // target_rank
            ,   params.dest_index       // target_disp
            ,   params.size_in_bytes    // target_count
            ,   MPI_BYTE                // target_datatype
            ,   win                     // win
            ,   &request                // request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Rput.\t"
            "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(params.src_ptr)
        ,   params.dest_rank
        ,   params.dest_index
        ,   params.size_in_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

// atomic operations

struct compare_and_swap_closure
{
    compare_and_swap_params params;
    MPI_Win                 win;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
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
            ,   win                                     // win
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
        
        MPI_Win_flush_all(win); // FIXME
        
        mgbase::execute(params.on_complete);
        
        return true;
    }
};

struct fetch_and_op_closure
{
    fetch_and_op_params     params;
    MPI_Win                 win;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
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
            ,   win                                 // win
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
        
        MPI_Win_flush_all(win); // FIXME
        
        mgbase::execute(params.on_complete);
        
        return true;
    }
};

// registration

struct attach_closure
{
    attach_async_params params;
    MPI_Win             win;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        const auto& p = params.base;
        
        MGBASE_ASSERT(p.ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(p.size > 0);
        
        MGBASE_LOG_DEBUG(
            "msg:Attach a memory region.\tptr:{:x}\tsize:{}"
        ,   reinterpret_cast<mgbase::uint64_t>(p.ptr)
        ,   p.size
        );
        
        mpi3_error::check(
            MPI_Win_attach(win, p.ptr, p.size)
        );
        
        mpi3_error::check(
            MPI_Get_address(p.ptr, params.addr_result)
        );
        
        mgbase::execute(params.on_complete);
        
        return true;
    }
};

struct detach_closure
{
    detach_async_params params;
    MPI_Win             win;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator () () const
    {
        MGBASE_ASSERT(params.base.ptr != MGBASE_NULLPTR);
        
        MGBASE_LOG_DEBUG(
            "msg:Detach a memory region.\tptr:{:x}"
        ,   reinterpret_cast<mgbase::uint64_t>(params.base.ptr)
        );
        
        mpi3_error::check(
            MPI_Win_detach(win, params.base.ptr)
        );
        
        mgbase::execute(params.on_complete);
        
        return true;
    }
};

// collective operations

struct ibarrier_closure
{
    ibarrier_params             params;
    mpi::mpi_completer_base*    comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        const auto& p = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Ibarrier(
                p.comm // TODO
            ,   &request
            )
        );
        
        MGBASE_LOG_DEBUG(
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

struct ibcast_closure
{
    ibcast_params            params;
    mpi::mpi_completer_base* comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        const auto& p = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Ibcast(
                p.coll.ptr
            ,   static_cast<int>(p.coll.num_bytes)
            ,   MPI_BYTE
            ,   static_cast<int>(p.coll.root)
            ,   p.comm
            ,   &request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Ibcast.\t"
            "root:{}\tptr:{:x}\tsize_in_bytes:{}"
        ,   p.coll.root
        ,   reinterpret_cast<mgbase::intptr_t>(p.coll.ptr)
        ,   p.coll.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct iallgather_closure
{
    iallgather_params           params;
    mpi::mpi_completer_base*    comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        const auto& p = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Iallgather(
                p.coll.src
            ,   static_cast<int>(p.coll.num_bytes)
            ,   MPI_BYTE
            ,   p.coll.dest
            ,   static_cast<int>(p.coll.num_bytes)
            ,   MPI_BYTE
            ,   p.comm
            ,   &request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Iallgather.\t"
            "src:{}\tdest:\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(p.coll.src)
        ,   reinterpret_cast<mgbase::intptr_t>(p.coll.dest)
        ,   p.coll.num_bytes
        );
        
        comp->complete(mpi::mpi_completer_base::complete_params{
            request
        ,   MPI_STATUS_IGNORE
        ,   params.on_complete
        });
        
        return true;
    }
};

struct ialltoall_closure
{
    ialltoall_params            params;
    mpi::mpi_completer_base*    comp;
    
    MGBASE_WARN_UNUSED_RESULT
    bool operator() () const
    {
        if (MGBASE_UNLIKELY(comp->full()))
            return false;
        
        const auto& p = params.base;
        
        MPI_Request request;
        
        mpi_error::check(
            MPI_Ialltoall(
                p.coll.src
            ,   static_cast<int>(p.coll.num_bytes)
            ,   MPI_BYTE
            ,   p.coll.dest
            ,   static_cast<int>(p.coll.num_bytes)
            ,   MPI_BYTE
            ,   p.comm
            ,   &request
            )
        );
        
        MGBASE_LOG_DEBUG(
            "msg:Executed MPI_Ialltoall.\t"
            "src:{}\tdest:\tsize_in_bytes:{}"
        ,   reinterpret_cast<mgbase::intptr_t>(p.coll.src)
        ,   reinterpret_cast<mgbase::intptr_t>(p.coll.dest)
        ,   p.coll.num_bytes
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

bool mpi3_delegator::try_rget(const rget_params& params)
{
    MGBASE_ASSERT(params.dest_ptr != MGBASE_NULLPTR);
    MGBASE_ASSERT(mpi::is_valid_rank(params.src_rank));
    MGBASE_ASSERT(params.size_in_bytes > 0);
    
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MGBASE_ASSERT(params.src_index != 0);
    
    const auto ret = try_delegate(
        this->get_delegator()
    ,   rget_closure{ params, &this->get_completer(), win_ }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tsize_in_bytes:{}"
    ,   (ret ? "Delegated MPI_Rget." : "Failed to delegate MPI_Rget.")
    ,   params.src_rank
    ,   params.src_index
    ,   reinterpret_cast<mgbase::intptr_t>(params.dest_ptr)
    ,   params.size_in_bytes
    );
    
    return ret;
}

bool mpi3_delegator::try_rput(const rput_params& params)
{
    MGBASE_ASSERT(params.src_ptr != MGBASE_NULLPTR);
    MGBASE_ASSERT(mpi::is_valid_rank(params.dest_rank));
    MGBASE_ASSERT(params.size_in_bytes > 0);
    
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MGBASE_ASSERT(params.dest_index != 0);
    
    const auto ret = try_delegate(
        this->get_delegator()
    ,   rput_closure{ params, &this->get_completer(), win_ }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tsize_in_bytes:{}"
    ,   (ret ? "Delegated MPI_Put." : "Failed to delegate MPI_Put.")
    ,   reinterpret_cast<mgbase::intptr_t>(params.src_ptr)
    ,   params.dest_rank
    ,   params.dest_index
    ,   params.size_in_bytes
    );
    
    return ret;
}

// atomic operations

bool mpi3_delegator::try_compare_and_swap(const compare_and_swap_params& params)
{
    MGBASE_ASSERT(mpi::is_valid_rank(params.dest_rank));
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MGBASE_ASSERT(params.dest_index != 0);
    
    const auto ret = try_delegate(
        this->get_delegator()
    ,   compare_and_swap_closure{ params, win_ }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "desired:{}\texpected:{}\tresult_ptr:{:x}\t"
        "datatype:{}\tdest_rank:{}\tdest_index:{:x}"
    ,   (ret ? "Delegated MPI_Compare_and_swap." : "Failed to delegate MPI_Compare_and_swap.")
    ,   params.desired
    ,   params.expected
    ,   reinterpret_cast<mgbase::intptr_t>(params.result_ptr)
    ,   get_datatype_name(params.datatype)
    ,   params.dest_rank
    ,   params.dest_index
    );
    
    return ret;
}

bool mpi3_delegator::try_fetch_and_op(const fetch_and_op_params& params)
{
    MGBASE_ASSERT(mpi::is_valid_rank(params.dest_rank));
    // TODO: This assertion might not be compliant to the MPI standard,
    //       but it's helpful to check whether the pointer is not null
    MGBASE_ASSERT(params.dest_index != 0);
       
    const auto ret = try_delegate(
        this->get_delegator()
    ,   fetch_and_op_closure{ params, win_ }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "value:{}\tresult_ptr:{:x}\t"
        "dest_rank:{}\tdest_index:{:x}\tdatatype:{}\toperation:{}"
    ,   (ret ? "Delegated MPI_Fetch_and_op." : "Failed to delegate MPI_Fetch_and_op.")
    ,   params.value
    ,   reinterpret_cast<mgbase::intptr_t>(params.result_ptr)
    ,   params.dest_rank
    ,   params.dest_index
    ,   get_datatype_name(params.datatype)
    ,   mgbase::force_integer_cast<mgbase::intptr_t>(params.operation)
    );
    
    return ret;
}

// registration

bool mpi3_delegator::try_attach_async(const attach_async_params& params)
{
    return try_delegate(
        this->get_delegator()
    ,   attach_closure{ params, win_ }
    );
}

bool mpi3_delegator::try_detach_async(const detach_async_params& params)
{
    return try_delegate(
        this->get_delegator()
    ,   detach_closure{ params, win_ }
    );
}

// collective operations

bool mpi3_delegator::try_ibarrier(const ibarrier_params& params)
{
    const auto ret = try_delegate(
        this->get_delegator()
    ,   ibarrier_closure{ params, &this->get_completer() }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}"
    ,   (ret ? "Delegated MPI_Ibarrier." : "Failed to delegate MPI_Ibarrier.")
    );
    
    return ret;
}

bool mpi3_delegator::try_ibcast(const ibcast_params& params)
{
    const auto ret = try_delegate(
        this->get_delegator()
    ,   ibcast_closure{ params, &this->get_completer() }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "root:{}\tptr:\tsize_in_bytes:{}"
    ,   (ret ? "Delegated MPI_Ibcast." : "Failed to delegate MPI_Ibcast.")
    ,   params.base.coll.root
    ,   reinterpret_cast<mgbase::intptr_t>(params.base.coll.ptr)
    ,   params.base.coll.num_bytes
    );
    
    return ret;
}

bool mpi3_delegator::try_iallgather(const iallgather_params& params)
{
    const auto ret = try_delegate(
        this->get_delegator()
    ,   iallgather_closure{ params, &this->get_completer() }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "src:{}\tdest:\tsize_in_bytes:{}"
    ,   (ret ? "Delegated MPI_Iallgather." : "Failed to delegate MPI_Iallgather.")
    ,   reinterpret_cast<mgbase::intptr_t>(params.base.coll.src)
    ,   reinterpret_cast<mgbase::intptr_t>(params.base.coll.dest)
    ,   params.base.coll.num_bytes
    );
    
    return ret;
}

bool mpi3_delegator::try_ialltoall(const ialltoall_params& params)
{
    const auto ret = try_delegate(
        this->get_delegator()
    ,   ialltoall_closure{ params, &this->get_completer() }
    );
    
    MGBASE_LOG_DEBUG(
        "msg:{}\t"
        "root:{}\tptr:\tsize_in_bytes:{}"
    ,   (ret ? "Queued MPI_Ialltoall." : "Failed to queue MPI_Ialltoall.")
    ,   reinterpret_cast<mgbase::intptr_t>(params.base.coll.src)
    ,   reinterpret_cast<mgbase::intptr_t>(params.base.coll.dest)
    ,   params.base.coll.num_bytes
    );
    
    return ret;
}

bool mpi3_delegator::try_native_barrier_async(const ibarrier_params& params)
{
    return this->try_ibarrier(params);
}

bool mpi3_delegator::try_native_broadcast_async(const ibcast_params& params)
{
    return this->try_ibcast(params);
}

bool mpi3_delegator::try_native_allgather_async(const iallgather_params& params)
{
    return this->try_iallgather(params);
}

bool mpi3_delegator::try_native_alltoall_async(const ialltoall_params& params)
{
    return this->try_ialltoall(params);
}

} // namespace mpi3
} // namespace mgcom

