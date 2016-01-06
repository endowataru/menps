
#include "unittest.hpp"
#include <mgbase/binded_function.hpp>

void assign(int& x, const int& y) {
    x = y;
}

TEST(BindedFunction, Basic)
{
    int x = 0;
    mgbase::binded_function<void (const int&)> func =
        mgbase::make_binded_function<void (int&, const int&), assign>(&x);
    
    func(123);
    
    ASSERT_EQ(123, x);
}

