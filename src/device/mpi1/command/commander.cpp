
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
    impl()
        : command_queue()
        , command_consumer()
        , command_producer(command_consumer::get_completer()) { }
    
    impl(const impl&) = delete;
    
private:
    
};


commander::commander()
    : impl_{mgbase::make_unique<impl>()} { }

commander::~commander() = default;

mgbase::unique_ptr<commander> make_commander()
{
    return mgbase::make_unique<commander>();
}

mpi::mpi_interface& commander::get_mpi_interface() MGBASE_NOEXCEPT
{
    return impl_->get_mpi_interface();
}

} // namespace mpi1
} // namespace mgcom

