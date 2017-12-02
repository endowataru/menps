
#include "unittest.hpp"
#include <menps/mecom2/rpc/mpi/mpi.hpp>

mefdn::unique_ptr<medev2::mpi::direct_requester> g_mi;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    
    g_mi = mefdn::make_unique<
        medev2::mpi::direct_requester>(&argc, &argv);
    
    const int ret = RUN_ALL_TESTS();
    
    return ret;
}

