
#include "unittest.hpp"
#include <mefdn/callback.hpp>
#include <mefdn/functional/functional_constant.hpp>
#include <functional>

namespace /*unnamed*/ {

void func1(int& x)
{
    x = 123;
}

#if 0
void func2(int a, int& x)
{
    x = a;
}
#endif

} // unnamed namespace

TEST(Callback, Base)
{
    //using hoge = typename std::result_of<decltype(&func1) (int&)>::type;
    
    mefdn::callback<void (int&)>
        f = &func1;
        //f = mefdn::make_callback(func1);
    
    int x;
    f(x);
    
    ASSERT_EQ(123, x);
    
    #if 0
    f = mefdn::make_callback(
        std::bind(MEFDN_FUNCTIONAL_CONSTANT(&func2), 100, std::placeholders::_1)
    );
    
    f(x);
    
    ASSERT_EQ(100, x);
    #endif
    
}



