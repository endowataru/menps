
#pragma once

#include "rma.hpp"
#include <mgcom/rma/requester.hpp>
#include "device/mpi/mpi_interface.hpp"

namespace mgcom {
namespace mpi {

class emulated_contiguous_requester
    : public virtual rma::requester
{
protected:
    emulated_contiguous_requester(rpc::requester&, mpi_interface&);
    
public:
    virtual ~emulated_contiguous_requester();
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_read(const rma::untyped::read_params&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_write(const rma::untyped::write_params&) MGBASE_OVERRIDE;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace mgcom

