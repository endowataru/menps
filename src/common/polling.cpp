
#include <mgcom.hpp>

namespace mgbase {
namespace control {

void yield()
{
    mgcom::rma::poll();
    mgcom::am::poll();
}

} // namespace control
} // namespace mgbase

