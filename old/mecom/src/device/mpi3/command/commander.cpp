
#include "command_producer.hpp"
#include "command_consumer.hpp"
#include "commander.hpp"

namespace menps {
namespace mecom {
namespace mpi3 {

class commander::impl
    : public command_consumer
    , public command_producer
{
public:
    explicit impl(endpoint& ep)
        : command_queue()
        , command_consumer()
        , command_producer(ep, command_consumer::get_completer())
    { }
    
    impl(const impl&) = delete;
    
private:
    
};


commander::commander(endpoint& ep)
    : impl_{mefdn::make_unique<impl>(ep)} { }

commander::~commander() = default;

mefdn::unique_ptr<commander> make_commander(endpoint& ep)
{
    return mefdn::make_unique<commander>(ep);
}

mpi3_interface& commander::get_mpi_interface() noexcept
{
    return impl_->get_mpi_interface();
}

rma_window& commander::get_win() noexcept
{
    return impl_->get_win();
}

} // namespace mpi3
} // namespace mecom
} // namespace menps

