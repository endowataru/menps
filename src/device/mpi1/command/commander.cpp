
#include "command_producer.hpp"
#include "command_consumer.hpp"
#include "commander.hpp"

namespace mgcom {
namespace mpi1 {

class commander::impl
    : protected command_consumer
    , public command_producer
{
public:
    explicit impl(endpoint& ep)
        : command_queue()
        , command_consumer()
        , command_producer(ep, command_consumer::get_completer()) { }
    
    impl(const impl&) = delete;
    
private:
    
};


commander::commander(endpoint& ep)
    : impl_{mgbase::make_unique<impl>(ep)} { }

commander::~commander() = default;

mgbase::unique_ptr<commander> make_commander(endpoint& ep)
{
    return mgbase::make_unique<commander>(ep);
}

mpi::mpi_interface& commander::get_mpi_interface() MGBASE_NOEXCEPT
{
    return impl_->get_mpi_interface();
}

} // namespace mpi1
} // namespace mgcom

