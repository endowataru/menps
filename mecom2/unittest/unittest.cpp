
#include "unittest.hpp"
#include <menps/mecom2/rpc/mpi/mpi.hpp>

medev2::mpi::direct_requester* g_mi;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    
    auto mi =
        mefdn::make_unique<medev2::mpi::direct_requester>(&argc, &argv);
    
    g_mi = mi.get();
    
    const int ret = RUN_ALL_TESTS();
    
    g_mi = nullptr;
    
    return ret;
}

