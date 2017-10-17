
#include "unittest.hpp"
#include <mefdn/functional/callback_function.hpp>
#include <mefdn/functional/bind_arg.hpp>
#include <mefdn/functional/make_callback_function.hpp>

namespace /*unnamed*/ {

void func1(int& x)
{
    x = 123;
}

} // unnamed namespace

TEST(CallbackFunction, Basic1)
{
    const mefdn::callback_function<void (int&)>
        f = mefdn::make_callback_function(
            MEFDN_MAKE_INLINED_FUNCTION(func1)
        );
    
    int x;
    f(x);
    
    ASSERT_EQ(123, x);
}

namespace /*unnamed*/ {

int func2()
{
    return 123;
}

} // unnamed namespace

TEST(CallbackFunction, Basic2)
{
    const mefdn::callback_function<int ()>
        f = mefdn::make_callback_function(
            MEFDN_MAKE_INLINED_FUNCTION(func2)
        );
    
    const int x = f();
    
    ASSERT_EQ(123, x);
}

namespace /*unnamed*/ {

int func3(int& x)
{
    return x + 1;
}

} // unnamed namespace

TEST(CallbackFunction, Bind1)
{
    int y;
    
    mefdn::callback_function<int ()>
        f = mefdn::make_callback_function(
            mefdn::bind1st_of_1(
                MEFDN_MAKE_INLINED_FUNCTION(func3)
            ,   mefdn::wrap_reference(y)
            )
        );
    
    y = 123;
    
    const int x = f();
    ASSERT_EQ(124, x);
    
    y = 1;
    
    const int x2 = mefdn::bind1st_of_1(
        MEFDN_MAKE_INLINED_FUNCTION(func3)
    ,   mefdn::wrap_reference(y)
    )();
    
    ASSERT_EQ(2, x2);
    
    //MEFDN_STATIC_ASSERT((mefdn::is_callable<mefdn::inlined_function<int (*)(int&), &func3> (int&)>::value));
}

namespace /*unnamed*/ {

int func4(int& x, int y)
{
    return x += y;
}

} // unnamed namespace

TEST(CallbackFunction, Bind2)
{
    int x;
    
    mefdn::callback_function<int (int)>
        f = mefdn::make_callback_function(
            mefdn::bind1st_of_2(
                MEFDN_MAKE_INLINED_FUNCTION(func4)
            ,   mefdn::wrap_reference(x)
            )
        );
    
    x = 123;
    const int r = f(3);
    
    ASSERT_EQ(126, x);
    ASSERT_EQ(126, r);
    
}


