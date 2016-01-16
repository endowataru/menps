
#include "unittest.hpp"

TEST(Base, Startup)
{

}

int main(int argc, char* argv[])
{
    ::testing::InitGoogleTest(&argc, argv);
    
    mgcom::initialize(&argc, &argv);
    
    const int ret = RUN_ALL_TESTS();
    
    mgcom::finalize();
    
    return ret;
}

