
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
    mpi3_requester(mpi3::mpi3_interface& mi, mpi3::rma_window& win)
        : mi_(mi)
        , win_(win) { }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_read_async(const untyped::read_params& params) MGBASE_OVERRIDE
    {
        return mi_.try_rget({
            untyped::to_raw_pointer(params.dest_laddr)
        ,   static_cast<int>(params.src_proc)
        ,   reinterpret_cast<MPI_Aint>(untyped::to_raw_pointer(params.src_raddr))
        ,   static_cast<int>(params.size_in_bytes)
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_write_async(const untyped::write_params& params) MGBASE_OVERRIDE
    {
        return mi_.try_rput({
            untyped::to_raw_pointer(params.src_laddr)
        ,   static_cast<int>(params.dest_proc)
        ,   reinterpret_cast<MPI_Aint>(untyped::to_raw_pointer(params.dest_raddr))
        ,   static_cast<int>(params.size_in_bytes)
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_atomic_read_async(const atomic_read_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mi_.try_fetch_and_op({
            0 // parameter "value" is unused
        ,   params.dest_ptr
        ,   mpi3::mpi_type<atomic_default_t>::datatype()
        ,   static_cast<int>(params.src_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.src_rptr))
        ,   MPI_NO_OP
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_atomic_write_async(const atomic_write_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mi_.try_fetch_and_op({
            params.value
        ,   MGBASE_NULLPTR
        ,   mpi3::mpi_type<atomic_default_t>::datatype()
        ,   static_cast<int>(params.dest_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.dest_rptr))
        ,   MPI_REPLACE
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_compare_and_swap_async(const compare_and_swap_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mi_.try_compare_and_swap({
            params.expected
        ,   params.desired
        ,   params.result_ptr
        ,   mpi3::mpi_type<atomic_default_t>::datatype()
        ,   static_cast<int>(params.target_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.target_rptr))
        ,   params.on_complete
        });
    }
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_fetch_and_add_async(const fetch_and_add_params<atomic_default_t>& params) MGBASE_OVERRIDE
    {
        return mi_.try_fetch_and_op({
            params.value
        ,   params.result_ptr
        ,   mpi3::mpi_type<atomic_default_t>::datatype()
        ,   static_cast<int>(params.target_proc)
        ,   reinterpret_cast<MPI_Aint>(to_raw_pointer(params.target_rptr))
        ,   MPI_SUM
        ,   params.on_complete
        });
    }
    
private:
    mpi3::mpi3_interface& mi_;
    mpi3::rma_window& win_;
};

} // unnamed namespace
} // namespace rma

namespace mpi3 {

mgbase::unique_ptr<rma::requester> make_rma_requester(mpi3_interface& mi, rma_window& win)
{
    return mgbase::make_unique<rma::mpi3_requester>(mi, win);
}

} // namespace mpi3

} // namespace mgcom

