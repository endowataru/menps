
#include "unittest.hpp"
#include <mgbase/deferred.hpp>

int f(int* x) {
    *x = 123;
    return 0;
}

/*TEST(Deferred, Cont)
{
    int val = 0;
    mgbase::deferred<int*> d = mgbase::make_ready_deferred(&val);
    mgbase::continuation<void> c;
    mgbase::deferred<void> d2 = mgbase::add_continuation<int (int*), f>(c, d);
    
    ASSERT_EQ(123, val);
}*/

struct B {
    MGBASE_CONTINUATION(void) cont;
    int x;
};

struct A {
    MGBASE_CONTINUATION(void) cont;
    B b;
    int x;
};

mgbase::deferred<void> g1(B& b) {
    b.x += 10;
    return mgbase::make_ready_deferred();
}

mgbase::deferred<void> g0(B& b) {
    b.x += 1;
    return mgbase::make_deferred<mgbase::deferred<void> (B&), g1>(b);
}

mgbase::deferred<void> f2(A& a) {
    a.x += 100;
    return mgbase::make_ready_deferred();
}

mgbase::deferred<void> f1(A& a) {
    a.x += 10;
    return mgbase::add_continuation<mgbase::deferred<void> (A&), &f2>(
        a
    ,   g0(a.b)
    );
}

mgbase::deferred<void> f0(A& a) {
    a.x += 1;
    return mgbase::add_continuation<mgbase::deferred<void> (A&), &f1>(
        a
    ,   g0(a.b)
    );
}

mgbase::resumable set_true(bool& b, const mgbase::ready_deferred<void>&) {
    b = true;
    return mgbase::make_empty_resumable();
}

TEST(Deferred, Cont)
{
    A a;
    a.x = 0;
    a.b.x = 0;
    
    mgbase::deferred<void> d = f0(a);
    ASSERT_EQ(1, a.x);
    ASSERT_EQ(1, a.b.x);
    
    mgbase::resumable res = d.set_terminal();
    ASSERT_FALSE(res.empty());
    
    ASSERT_EQ(1, a.x);
    ASSERT_EQ(1, a.b.x);
    
    ASSERT_FALSE(d.resume());
    
    ASSERT_EQ(11, a.x);
    ASSERT_EQ(12, a.b.x);
    
    ASSERT_TRUE(d.resume());
    
    ASSERT_EQ(111, a.x);
    ASSERT_EQ(22, a.b.x);
}


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
        ,   mgbase::make_bound_function<mgbase::resumable (), infinite>()
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
        return mgbase::make_deferred<mgbase::deferred<void> (A&), func>(a);
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

TEST(Deferred, Wait)
{
    A a;
    a.x = 0;
    
    mgbase::deferred<void> d = func(a);
    d.wait();
    
    ASSERT_EQ(10, a.x);
    
    d = mgbase::make_ready_deferred();
    d.wait();
}

