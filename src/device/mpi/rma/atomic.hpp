
#pragma once

#include "rma.hpp"
#include <mgcom/rma/requester.hpp>
#include <mgbase/unique_ptr.hpp>

namespace mgcom {
namespace mpi {

class emulated_atomic_requester
    : public virtual rma::requester
{
protected:
    explicit emulated_atomic_requester(rpc::requester&);
    
public:
    virtual ~emulated_atomic_requester();
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_read(
        const rma::async_atomic_read_params<rma::atomic_default_t>&
    ) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_atomic_write(
        const rma::async_atomic_write_params<rma::atomic_default_t>&
    ) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_compare_and_swap(
        const rma::async_compare_and_swap_params<rma::atomic_default_t>&
    ) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual ult::async_status<void> async_fetch_and_add(
        const rma::async_fetch_and_add_params<rma::atomic_default_t>&
    ) MGBASE_OVERRIDE;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace mgcom

