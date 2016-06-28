
#include <mgcom.hpp>
#include "device/fjmpi/fjmpi.hpp"
#include "starter.hpp"

namespace mgcom {

namespace /*unnamed*/ {

mgbase::unique_ptr<starter> g_starter;

} // unnamed namespace

void initialize(int* const argc, char*** const argv)
{
    g_starter = fjmpi::make_starter(argc, argv);
}

void finalize()
{
    g_starter.reset();
}

endpoint* endpoint::endpoint_;

namespace rma {

requester* requester::req_;

registrator* registrator::reg_;

} // namespace rma

namespace rpc {

requester* requester::req_;

} // namespace rpc

namespace collective {

requester* requester::req_;

} // namespace collective

} // namespace mgcom

