
#include "unittest.hpp"
#include <mgbase/move.hpp>

struct A
{
    A() : count(0) { }
    A(MGBASE_RV_REF(A) a) : count(a.count + 1) { a.count = 0; }
    
    A& operator = (MGBASE_RV_REF(A) a) {
        count = a.count + 1;
        a.count = 0;
        return *this;
    }
    
    MGBASE_MOVABLE_BUT_NOT_COPYABLE(A)
    
    int count;
};

TEST(Move, Basic)
{
    A a;
    
    A a2(mgbase::move(a));
    
    A a3;
    a3 = mgbase::move(a2);
    
    ASSERT_EQ(0, a.count);
    ASSERT_EQ(0, a2.count);
    ASSERT_EQ(2, a3.count);
    //A a1 = mgbase::move(a);
}



