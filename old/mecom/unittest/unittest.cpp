
#include "unittest.hpp"

TEST(Base, Startup)
{
    // Do nothing.
}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    
    mecom::initialize(&argc, &argv);
    
    const int ret = RUN_ALL_TESTS();
    
    mecom::finalize();
    
    return ret;
}

