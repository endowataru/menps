
#include "unittest.hpp"
#include <mgbase/threading/thread.hpp>

namespace /*unnamed*/ {

int val;
void f() { val = 1; }

} // unnamed namespace

TEST(Thread, Basic)
{
    val = 0;
    
    mgbase::thread th(&f);
    th.join();
    
    ASSERT_EQ(1, val);
}
