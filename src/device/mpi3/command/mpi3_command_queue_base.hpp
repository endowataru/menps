
#pragma once

#include "mpi3_command.hpp"

namespace mgcom {
namespace mpi3 {

template <typename Derived, typename CommandCode>
class mpi3_command_queue_base
{
    typedef CommandCode     command_code_type;
    
protected:
    mpi3_command_queue_base() MGBASE_EMPTY_DEFINITION
    
public:
    bool try_get(
        void*                       dest_ptr
    ,   int                         src_rank
    ,   MPI_Aint                    src_index
    ,   int                         size_in_bytes
    ,   const mgbase::operation&    on_complete
    ) {
        MGBASE_ASSERT(dest_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(mpi::is_valid_rank(src_rank));
        MGBASE_ASSERT(size_in_bytes > 0);
        
        // TODO: This assertion might not be compliant to the MPI standard,
        //       but it's helpful to check whether the pointer is not null
        MGBASE_ASSERT(src_index != 0);
        
        const mpi3_command_parameters::get_parameters params = {
            dest_ptr
        ,   src_rank
        ,   src_index
        ,   size_in_bytes
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.get = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_GET
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "src_rank:{}\tsrc_index:{:x}\tdest_ptr:{:x}\tsize_in_bytes:{}"
        ,   (ret ? "Enqueued MPI_Get." : "Failed to queue MPI_Get.")
        ,   src_rank
        ,   src_index
        ,   reinterpret_cast<mgbase::intptr_t>(dest_ptr)
        ,   size_in_bytes
        );
        
        return ret;
    }
    
    bool try_put(
        const void*                 src_ptr
    ,   int                         dest_rank
    ,   MPI_Aint                    dest_index
    ,   int                         size_in_bytes
    ,   const mgbase::operation&    on_complete
    ) {
        MGBASE_ASSERT(src_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(mpi::is_valid_rank(dest_rank));
        MGBASE_ASSERT(size_in_bytes > 0);
        
        // TODO: This assertion might not be compliant to the MPI standard,
        //       but it's helpful to check whether the pointer is not null
        MGBASE_ASSERT(dest_index != 0);
        
        const mpi3_command_parameters::put_parameters params = {
            src_ptr
        ,   dest_rank
        ,   dest_index
        ,   size_in_bytes
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.put = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_PUT
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "src_ptr:{:x}\tdest_rank:{}\tdest_index:{:x}\tsize_in_bytes:{}"
        ,   (ret ? "Queued MPI_Put." : "Failed to queue MPI_Put.")
        ,   reinterpret_cast<mgbase::intptr_t>(src_ptr)
        ,   dest_rank
        ,   dest_index
        ,   size_in_bytes
        );
        
        return ret;
    }
    
    bool try_compare_and_swap(
        const void*                 expected_ptr
    ,   const void*                 desired_ptr
    ,   void*                       result_ptr
    ,   MPI_Datatype                datatype
    ,   int                         dest_rank
    ,   MPI_Aint                    dest_index
    ,   const mgbase::operation&    on_complete
    ) {
        MGBASE_ASSERT(expected_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(desired_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(mpi::is_valid_rank(dest_rank));
        // TODO: This assertion might not be compliant to the MPI standard,
        //       but it's helpful to check whether the pointer is not null
        MGBASE_ASSERT(dest_index != 0);
        
        const mpi3_command_parameters::compare_and_swap_parameters params = {
            expected_ptr
        ,   desired_ptr
        ,   result_ptr
        ,   datatype
        ,   dest_rank
        ,   dest_index
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.compare_and_swap = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_COMPARE_AND_SWAP
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "desired_ptr:{:x}\texpected_ptr:{:x}\tresult_ptr:{:x}\t"
            "datatype:{}\tdest_rank:{}\tdest_index:{:x}"
        ,   (ret ? "Queued MPI_Compare_and_swap." : "Failed to queue MPI_Compare_and_swap.")
        ,   reinterpret_cast<mgbase::intptr_t>(desired_ptr)
        ,   reinterpret_cast<mgbase::intptr_t>(expected_ptr)
        ,   reinterpret_cast<mgbase::intptr_t>(result_ptr)
        ,   detail::get_datatype_name(datatype)
        ,   dest_rank
        ,   dest_index
        );
        
        return ret;
    }
    
    bool try_fetch_and_op(
        const void*                 value_ptr
    ,   void*                       result_ptr
    ,   MPI_Datatype                datatype
    ,   int                         dest_rank
    ,   MPI_Aint                    dest_index
    ,   MPI_Op                      operation
    ,   const mgbase::operation&    on_complete
    ) {
        MGBASE_ASSERT(value_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(result_ptr != MGBASE_NULLPTR);
        MGBASE_ASSERT(mpi::is_valid_rank(dest_rank));
        // TODO: This assertion might not be compliant to the MPI standard,
        //       but it's helpful to check whether the pointer is not null
        MGBASE_ASSERT(dest_index != 0);
        
        const mpi3_command_parameters::fetch_and_op_parameters params = {
            value_ptr
        ,   result_ptr
        ,   datatype
        ,   dest_rank
        ,   dest_index
        ,   operation
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.fetch_and_op = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_FETCH_AND_OP
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "value_ptr:{:x}\tresult_ptr:{:x}\t"
            "dest_rank:{}\tdest_index:{:x}\tdatatype:{}\toperation:{}"
        ,   (ret ? "Queued MPI_Fetch_and_op." : "Failed to queue MPI_Fetch_and_op.")
        ,   reinterpret_cast<mgbase::intptr_t>(value_ptr)
        ,   reinterpret_cast<mgbase::intptr_t>(result_ptr)
        ,   dest_rank
        ,   dest_index
        ,   detail::get_datatype_name(datatype)
        ,   reinterpret_cast<mgbase::intptr_t>(operation)
        );
        
        return ret;
    }
    
    bool try_ibarrier(
        const MPI_Comm              comm
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi3_command_parameters::ibarrier_parameters params = {
            comm
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.ibarrier = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_IBARRIER
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}"
        ,   (ret ? "Queued MPI_Ibarrier." : "Failed to queue MPI_Ibarrier.")
        );
        
        return ret;
    }
    
    bool try_ibcast(
        const process_id_t          root
    ,   void* const                 ptr
    ,   const index_t               number_of_bytes
    ,   const MPI_Comm              comm
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi3_command_parameters::ibcast_parameters params = {
            root
        ,   ptr
        ,   number_of_bytes
        ,   comm
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.ibcast = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_IBCAST
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "root:{}\tptr:\tsize_in_bytes:{}"
        ,   (ret ? "Queued MPI_Ibcast." : "Failed to queue MPI_Ibcast.")
        ,   root
        ,   reinterpret_cast<mgbase::intptr_t>(ptr)
        ,   number_of_bytes
        );
        
        return ret;
    }
    
    bool try_iallgather(
        const void* const           src
    ,   void* const                 dest
    ,   const index_t               number_of_bytes
    ,   const MPI_Comm              comm
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi3_command_parameters::iallgather_parameters params = {
            src
        ,   dest
        ,   number_of_bytes
        ,   comm
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.iallgather = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_IALLGATHER
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "src:{}\tdest:\tsize_in_bytes:{}"
        ,   (ret ? "Queued MPI_Iallgather." : "Failed to queue MPI_Iallgather.")
        ,   reinterpret_cast<mgbase::intptr_t>(src)
        ,   reinterpret_cast<mgbase::intptr_t>(dest)
        ,   number_of_bytes
        );
        
        return ret;
    }
    
    bool try_ialltoall(
        const void* const           src
    ,   void* const                 dest
    ,   const index_t               number_of_bytes
    ,   const MPI_Comm              comm
    ,   const mgbase::operation&    on_complete
    ) {
        const mpi3_command_parameters::ialltoall_parameters params = {
            src
        ,   dest
        ,   number_of_bytes
        ,   comm
        ,   on_complete
        };
        
        mpi3_command_parameters mpi3_params;
        mpi3_params.ialltoall = params;
        
        const bool ret = derived().try_enqueue_mpi3(
            command_code_type::MPI3_COMMAND_IALLTOALL
        ,   mpi3_params
        );
        
        MGBASE_LOG_DEBUG(
            "msg:{}\t"
            "root:{}\tptr:\tsize_in_bytes:{}"
        ,   (ret ? "Queued MPI_Ialltoall." : "Failed to queue MPI_Ialltoall.")
        ,   reinterpret_cast<mgbase::intptr_t>(src)
        ,   reinterpret_cast<mgbase::intptr_t>(dest)
        ,   number_of_bytes
        );
        
        return ret;
    }

private:
    Derived& derived() { return static_cast<Derived&>(*this); }
};

} // namespace mpi3
} // namespace mgcom

