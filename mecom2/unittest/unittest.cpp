
#include "unittest.hpp"
#include <menps/mecom2/rpc/mpi/mpi.hpp>

direct_mpi_itf_t::mpi_facade_type* g_mi = nullptr;

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    
    auto mi =
        mefdn::make_unique<direct_mpi_itf_t::mpi_facade_type>(&argc, &argv);
    
    g_mi = mi.get();
    
    const int ret = RUN_ALL_TESTS();
    
    g_mi = nullptr;
    
    return ret;
}

