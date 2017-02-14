
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


ult::async_status<void> emulated_atomic_requester::async_atomic_read(const rma::async_atomic_read_params<rma::atomic_default_t>& params) {
    return impl_->atomic_read(params);
}

ult::async_status<void> emulated_atomic_requester::async_atomic_write(const rma::async_atomic_write_params<rma::atomic_default_t>& params) {
    return impl_->atomic_write(params);
}

ult::async_status<void> emulated_atomic_requester::async_compare_and_swap(const rma::async_compare_and_swap_params<rma::atomic_default_t>& params) {
    return impl_->compare_and_swap(params);
}

ult::async_status<void> emulated_atomic_requester::async_fetch_and_add(const rma::async_fetch_and_add_params<rma::atomic_default_t>& params) {
    return impl_->fetch_and_add(params);
}

} // namespace mpi
} // namespace mgcom

