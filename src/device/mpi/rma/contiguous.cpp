
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

bool emulated_contiguous_requester::try_read_async(const rma::untyped::read_params& params) {
    return impl_->try_read_async(params);
}

bool emulated_contiguous_requester::try_write_async(const rma::untyped::write_params& params) {
    return impl_->try_write_async(params);
}

} // namespace mpi
} // namespace mgcom

