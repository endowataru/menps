
#pragma once

#include "rma.hpp"
#include <mgcom/rpc/requester.hpp>
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
    virtual bool try_atomic_read_async(const rma::atomic_read_params<rma::atomic_default_t>&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_atomic_write_async(const rma::atomic_write_params<rma::atomic_default_t>&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_compare_and_swap_async(const rma::compare_and_swap_params<rma::atomic_default_t>&) MGBASE_OVERRIDE;
    
    MGBASE_WARN_UNUSED_RESULT
    virtual bool try_fetch_and_add_async(const rma::fetch_and_add_params<rma::atomic_default_t>&) MGBASE_OVERRIDE;
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

} // namespace mpi
} // namespace mgcom

