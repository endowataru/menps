
#include "command_producer.hpp"
#include "command_consumer.hpp"
#include "scheduler.hpp"
#include "device/mpi/command/mpi_delegator.hpp"
#include "fjmpi_delegator.hpp"

namespace mgcom {
namespace fjmpi {

class scheduler::impl
    : protected command_consumer
    , public command_producer
{
public:
    impl()
        : command_queue()
        , command_consumer()
        , command_producer()
        , mpi_{this->get_delegator(), command_consumer::get_mpi_completer()}
        , fjmpi_{*this} { }
    
    impl(const impl&) = delete;
    
    mpi::mpi_interface& get_mpi_interface() MGBASE_NOEXCEPT {
        return mpi_;
    }
    
    fjmpi_interface& get_fjmpi_interface() MGBASE_NOEXCEPT {
        return fjmpi_;
    }
    
private:
    mpi::mpi_delegator mpi_;
    fjmpi_delegator fjmpi_;
};


scheduler::scheduler()
    : impl_{mgbase::make_unique<impl>()} { }

scheduler::~scheduler() = default;

mgbase::unique_ptr<scheduler> make_scheduler()
{
    return mgbase::make_unique<scheduler>();
}

command_producer& scheduler::get_command_producer() MGBASE_NOEXCEPT
{
    return *impl_;
}

mpi::mpi_interface& scheduler::get_mpi_interface() MGBASE_NOEXCEPT
{
    return impl_->get_mpi_interface();
}

fjmpi_interface& scheduler::get_fjmpi_interface() MGBASE_NOEXCEPT
{
    return impl_->get_fjmpi_interface();
}

} // namespace fjmpi
} // namespace mgcom

