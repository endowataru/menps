
#include "unittest.hpp"
#include <mgbase/functional/bind.hpp>

#if 0
namespace /*unnamed*/ {

void assign(int& x, const int& y) {
    x = y;
}

} // unnamed namespace

TEST(Bind, Basic)
{
    int x = 0;
    auto func =
        mgbase::bind(&assign, mgbase::placeholders::_1, 123);
    
    func(x);
    
    ASSERT_EQ(123, x);
}
#endif

