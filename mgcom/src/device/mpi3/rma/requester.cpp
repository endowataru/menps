
#include "rma.hpp"
#include "common/rma/rma.hpp"
#include "device/mpi3/mpi3_type.hpp"

namespace mgcom {

namespace rma {
namespace /*unnamed*/ {

class mpi3_requester
    : public requester
{
public:
    /*implicit*/ mpi3_requester(mpi3::mpi3_interface& mi, const MPI_Win win)
        : mi_(mi)
        , win_(win)
    { }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_read(
        const untyped::read_params& params
    ) MGBASE_OVERRIDE
    {
        return mi_.get_async({
            {
                untyped::to_raw_pointer(params.dest_laddr)
            ,   static_cast<int>(params.src_proc)
            ,   reinterpret_cast<MPI_Aint>(untyped::to_raw_pointer(params.src_raddr))
            ,   static_cast<int>(params.size_in_bytes)
            ,   this->win_
            }
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_write(
        const untyped::write_params& params
    ) MGBASE_OVERRIDE
    {
        return mi_.put_async({
            {
                untyped::to_raw_pointer(params.src_laddr)
            ,   static_cast<int>(params.dest_proc)
            ,   reinterpret_cast<MPI_Aint>(untyped::to_raw_pointer(params.dest_raddr))
            ,   static_cast<int>(params.size_in_bytes)
            ,   this->win_
            }
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_read(
        const async_atomic_read_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        return mi_.fetch_and_op_async({
            {
                0 // parameter "value" is unused
            ,   params.dest_ptr
            ,   mpi3::mpi_type<atomic_default_t>::datatype()
            ,   static_cast<int>(params.src_proc)
            ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.src_rptr))
            ,   MPI_NO_OP
            ,   this->win_
            }
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_write(
        const async_atomic_write_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        return mi_.fetch_and_op_async({
            {
                params.value
            ,   MGBASE_NULLPTR
            ,   mpi3::mpi_type<atomic_default_t>::datatype()
            ,   static_cast<int>(params.dest_proc)
            ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.dest_rptr))
            ,   MPI_REPLACE
            ,   this->win_
            }
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_compare_and_swap(
        const async_compare_and_swap_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        return mi_.compare_and_swap_async({
            {
                params.expected
            ,   params.desired
            ,   params.result_ptr
            ,   mpi3::mpi_type<atomic_default_t>::datatype()
            ,   static_cast<int>(params.target_proc)
            ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.target_rptr))
            ,   this->win_
            }
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_fetch_and_add(
        const async_fetch_and_add_params<atomic_default_t>& params
    ) MGBASE_OVERRIDE
    {
        return mi_.fetch_and_op_async({
            {
                params.value
            ,   params.result_ptr
            ,   mpi3::mpi_type<atomic_default_t>::datatype()
            ,   static_cast<int>(params.target_proc)
            ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.target_rptr))
            ,   MPI_SUM
            ,   this->win_
            }
        ,   params.on_complete
        });
    }
    
private:
    mpi3::mpi3_interface&   mi_;
    const MPI_Win           win_;
};

} // unnamed namespace
} // namespace rma

namespace mpi3 {

mgbase::unique_ptr<rma::requester> make_rma_requester(mpi3_interface& mi, const MPI_Win win)
{
    return mgbase::make_unique<rma::mpi3_requester>(mi, win);
}

} // namespace mpi3

} // namespace mgcom

