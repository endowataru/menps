
#include <mgcom.hpp>
#include <cstdlib> // getenv
#include <stdexcept>
#include "device/mpi1/mpi1.hpp"

namespace mgcom {

namespace /*unnamed*/ {

mgbase::unique_ptr<starter> g_starter;

typedef mgbase::unique_ptr<starter> (*factory_func_t)(int*, char***);

struct device
{
    const char*     name;
    factory_func_t  func;
};

const device devs[] =
{
    { "mpi1", mpi1::make_starter }
};

factory_func_t select_starter()
{
    const char* const dev_name = getenv("MGCOM_DEVICE");
    
    if (dev_name != MGBASE_NULLPTR)
    {
        for (mgbase::size_t i = 0; i < sizeof(devs)/sizeof(devs[0]); ++i)
        {
            if (strcmp(dev_name, devs[i].name) == 0)
            {
                return devs[i].func;
            }
        }
    }
    
    return devs[0].func;
}

} // unnamed namespace



void initialize(int* const argc, char*** const argv)
{
    g_starter = select_starter()(argc, argv);
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

