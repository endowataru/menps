
#include "contiguous.impl.hpp"

namespace mgcom {
namespace mpi {

class emulated_contiguous_requester::impl
    : public emulated_contiguous
{
    typedef emulated_contiguous base;
    
public:
    impl(rpc::requester& req, mpi_interface& mi)
        : base{req, mi} { }
};

emulated_contiguous_requester::emulated_contiguous_requester(rpc::requester& req, mpi_interface& mi)
    : impl_{mgbase::make_unique<impl>(req, mi)} { }

emulated_contiguous_requester::~emulated_contiguous_requester() = default;

ult::async_status<void> emulated_contiguous_requester::async_read(const rma::untyped::read_params& params) {
    return impl_->read_async(params);
}

ult::async_status<void> emulated_contiguous_requester::async_write(const rma::untyped::write_params& params) {
    return impl_->write_async(params);
}

} // namespace mpi
} // namespace mgcom

