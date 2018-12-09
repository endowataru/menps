
#include "command_producer.hpp"
#include "command_consumer.hpp"
#include "scheduler.hpp"
#include "device/mpi/command/mpi_delegator.hpp"
#include "fjmpi_delegator.hpp"

namespace menps {
namespace mecom {
namespace fjmpi {

class scheduler::impl
    : protected command_consumer
    , public command_producer
{
public:
    impl(endpoint& ep)
        : command_queue()
        , command_consumer(ep)
        , command_producer()
        , mpi_{ep, this->get_delegator(), command_consumer::get_mpi_completer()}
        , fjmpi_{ep, *this} { }
    
    impl(const impl&) = delete;
    
    mpi::mpi_interface& get_mpi_interface() noexcept {
        return mpi_;
    }
    
    fjmpi_interface& get_fjmpi_interface() noexcept {
        return fjmpi_;
    }
    
private:
    mpi::mpi_delegator mpi_;
    fjmpi_delegator fjmpi_;
};


scheduler::scheduler(endpoint& ep)
    : impl_{mefdn::make_unique<impl>(ep)} { }

scheduler::~scheduler() = default;

mefdn::unique_ptr<scheduler> make_scheduler(endpoint& ep)
{
    return mefdn::make_unique<scheduler>(ep);
}

command_producer& scheduler::get_command_producer() noexcept
{
    return *impl_;
}

mpi::mpi_interface& scheduler::get_mpi_interface() noexcept
{
    return impl_->get_mpi_interface();
}

fjmpi_interface& scheduler::get_fjmpi_interface() noexcept
{
    return impl_->get_fjmpi_interface();
}

} // namespace fjmpi
} // namespace mecom
} // namespace menps

