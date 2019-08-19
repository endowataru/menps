
#include "unittest.hpp"
#include <menps/mefdn/tuple.hpp>
#include <menps/mefdn/tuple/apply.hpp>

#if 0

TEST(Tuple, Basic)
{
    mefdn::tuple<int> x(123);
    ASSERT_EQ(123, mefdn::get<0>(x));
}

TEST(Tuple, MakeTuple)
{
    auto t = mefdn::make_tuple(123);
    ASSERT_EQ(123, mefdn::get<0>(t));
}

TEST(Tuple, ForwardAsTuple)
{
    int x = 0;
    auto t = mefdn::forward_as_tuple(x);
    
    mefdn::get<0>(t) = 123;
    
    ASSERT_EQ(123, x);
}

#endif

namespace /*unnamed*/ {

int plus(int x, int y)
{
    return x + y;
}

} // unnamed namespace

TEST(Tuple, Apply)
{
    auto i = menps::mefdn::apply(plus, menps::mefdn::make_tuple(100, 200));
    ASSERT_EQ(300, i);
}

