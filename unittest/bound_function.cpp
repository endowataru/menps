
#include "unittest.hpp"
#include <mgbase/bound_function.hpp>

#include <mgbase/functional/inlined_function.hpp>

namespace /*unnamed*/ {

void assign(int& x, const int& y) {
    x = y;
}

} // unnamed namespace

TEST(BoundFunction, Basic)
{
    int x = 0;
    mgbase::bound_function<void (const int&)> func =
        mgbase::make_bound_function<void (int&, const int&), assign>(&x);
    
    func(123);
    
    ASSERT_EQ(123, x);
}

namespace /*unnamed*/ {

void f(int& x)
{
    x = 123;
}

} // unnamed namespace

TEST(InlinedFunction, Basic)
{
    mgbase::inlined_function<void (*)(int&), &f> x = MGBASE_MAKE_INLINED_FUNCTION(f);
    int val;
    x(val);
    
    ASSERT_EQ(123, val);
}

