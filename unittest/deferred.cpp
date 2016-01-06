
#include "unittest.hpp"
#include <mgbase/deferred.hpp>

int f(int* x) {
    *x = 123;
    return 0;
}

TEST(Deferred, Cont)
{
    int val = 0;
    mgbase::deferred<int*> d = mgbase::make_ready_deferred(&val);
    mgbase::continuation<void> c;
    mgbase::deferred<void> d2 = mgbase::add_continuation<int (int*), f>(c, d);
    
    ASSERT_EQ(123, val);
}

mgbase::resumable infinite() {
    return mgbase::make_binded_function<mgbase::resumable (), infinite>();
}

struct A {
    MGBASE_CONTINUATION(void) cont;
    int x;
};

/*
TEST(Deferred, Lazy)
{
    int x = 0;
    mgbase::continuation<int*> c;
    mgbase::continuation<void> c2;
    mgbase::add_continuation<int (int*), f>(
        c2
    ,   mgbase::make_deferred(
            c
        ,   mgbase::make_binded_function<mgbase::resumable (), infinite>()
        )
    );
    
    ASSERT_EQ(0, x);
    
    c.call(mgbase::make_ready_deferred(&x));
    
    ASSERT_EQ(123, x);
}
*/


mgbase::deferred<void> func(A& a)
{
    if (a.x < 10) {
        ++a.x;
        return mgbase::make_deferred<void, A, &A::cont, &func>(a);
    }
    else
        return mgbase::make_ready_deferred();
}

TEST(Deferred, Resume)
{
    A a;
    a.x = 0;
    
    mgbase::deferred<void> d = func(a);
    for (int i = 1; i < 10; i++) {
        ASSERT_EQ(i, a.x);
        d.resume();
        //ASSERT_FALSE(r.is_ready());
    }
    
    ASSERT_EQ(10, a.x);
    //ASSERT_TRUE(r.is_ready());
}

