
#include "unittest.hpp"
#include <mgbase/callback.hpp>
#include <mgbase/functional/functional_constant.hpp>
#include <functional>

namespace /*unnamed*/ {

void func1(int& x)
{
    x = 123;
}

void func2(int a, int& x)
{
    x = a;
}

} // unnamed namespace

TEST(Callback, Base)
{
    //using hoge = typename std::result_of<decltype(&func1) (int&)>::type;
    
    mgbase::callback<void (int&)>
        f = &func1;
        //f = mgbase::make_callback(func1);
    
    int x;
    f(x);
    
    ASSERT_EQ(123, x);
    
    #if 0
    f = mgbase::make_callback(
        std::bind(MGBASE_FUNCTIONAL_CONSTANT(&func2), 100, std::placeholders::_1)
    );
    
    f(x);
    
    ASSERT_EQ(100, x);
    #endif
    
}



