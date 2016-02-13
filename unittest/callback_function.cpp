
#include "unittest.hpp"
#include <mgbase/functional/callback_function.hpp>
#include <mgbase/functional/bind_arg.hpp>
#include <mgbase/functional/make_callback_function.hpp>

namespace /*unnamed*/ {

void func1(int& x)
{
    x = 123;
}

} // unnamed namespace

TEST(CallbackFunction, Basic1)
{
    const mgbase::callback_function<void (int&)>
        f = mgbase::make_callback_function(
            MGBASE_MAKE_INLINED_FUNCTION(func1)
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
    const mgbase::callback_function<int ()>
        f = mgbase::make_callback_function(
            MGBASE_MAKE_INLINED_FUNCTION(func2)
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
    
    mgbase::callback_function<int ()>
        f = mgbase::make_callback_function(
            mgbase::bind_ref1(
                MGBASE_MAKE_INLINED_FUNCTION(func3)
            ,   y
            )
        );
    
    y = 123;
    
    const int x = f();
    ASSERT_EQ(124, x);
    
    y = 1;
    
    const int x2 = mgbase::bind_ref1(
        MGBASE_MAKE_INLINED_FUNCTION(func3)
    ,   y
    )();
    
    ASSERT_EQ(2, x2);
    
    MGBASE_STATIC_ASSERT((mgbase::is_callable<mgbase::inlined_function<int (*)(int&), &func3> (int&)>::value));
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
    
    mgbase::callback_function<int (int)>
        f = mgbase::make_callback_function(
            mgbase::bind_ref1(
                MGBASE_MAKE_INLINED_FUNCTION(func4)
            ,   x
            )
        );
    
    x = 123;
    const int r = f(3);
    
    ASSERT_EQ(126, x);
    ASSERT_EQ(126, r);
    
}


