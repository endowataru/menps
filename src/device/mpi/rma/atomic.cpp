
#include "atomic.impl.hpp"

namespace mgcom {
namespace mpi {

class emulated_atomic_requester::impl
    : public emulated_atomic<rma::atomic_default_t>
{
    typedef emulated_atomic<rma::atomic_default_t>  base;
    
public:
    explicit impl(rpc::requester& req)
        : base(req) { }
};

emulated_atomic_requester::emulated_atomic_requester(rpc::requester& req)
    : impl_{mgbase::make_unique<impl>(req)} { }

emulated_atomic_requester::~emulated_atomic_requester() = default;


bool emulated_atomic_requester::try_atomic_read_async(const rma::atomic_read_params<rma::atomic_default_t>& params) {
    return impl_->try_read(params);
}

bool emulated_atomic_requester::try_atomic_write_async(const rma::atomic_write_params<rma::atomic_default_t>& params) {
    return impl_->try_write(params);
}

bool emulated_atomic_requester::try_compare_and_swap_async(const rma::compare_and_swap_params<rma::atomic_default_t>& params) {
    return impl_->try_compare_and_swap(params);
}

bool emulated_atomic_requester::try_fetch_and_add_async(const rma::fetch_and_add_params<rma::atomic_default_t>& params) {
    return impl_->try_fetch_and_add(params);
}

} // namespace mpi
} // namespace mgcom

