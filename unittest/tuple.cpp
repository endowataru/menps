
#include "unittest.hpp"
#include <mgbase/tuple.hpp>
#include <mgbase/tuple/apply.hpp>

TEST(Tuple, Basic)
{
    mgbase::tuple<int> x(123);
    ASSERT_EQ(123, mgbase::get<0>(x));
}

TEST(Tuple, MakeTuple)
{
    auto t = mgbase::make_tuple(123);
    ASSERT_EQ(123, mgbase::get<0>(t));
}

TEST(Tuple, ForwardAsTuple)
{
    int x = 0;
    auto t = mgbase::forward_as_tuple(x);
    
    mgbase::get<0>(t) = 123;
    
    ASSERT_EQ(123, x);
}

namespace /*unnamed*/ {

int plus(int x, int y)
{
    return x + y;
}

} // unnamed namespace

TEST(Tuple, Apply)
{
    auto i = mgbase::apply(plus, mgbase::make_tuple(100, 200));
    ASSERT_EQ(300, i);
}

