
#include "unittest.hpp"
#include <mefdn/deferred.hpp>

struct B {
    MEFDN_CONTINUATION(void) cont;
    int x;
};

struct A {
    MEFDN_CONTINUATION(void) cont;
    B b;
    int x;
};

namespace /*unnamed*/ {

inline mefdn::deferred<void> g1(B& b) {
    b.x += 10;
    // This function immediately finishes.
    return mefdn::make_ready_deferred();
}

inline mefdn::deferred<void> g0(B& b) {
    b.x += 1;
    // Call g1 when the function is resumed.
    return mefdn::make_deferred(MEFDN_MAKE_INLINED_FUNCTION(&g1), b);
}

inline mefdn::deferred<void> f2(A& a) {
    a.x += 100;
    // This function immediately finishes.
    return mefdn::make_ready_deferred();
}

inline mefdn::deferred<void> f1(A& a) {
    a.x += 10;
    // Call f2 when g0 finishes.
    return g0(a.b)
        .add_continuation(
            MEFDN_MAKE_INLINED_FUNCTION(&f2)
        ,   a
        );
    
}

inline mefdn::deferred<void> f0(A& a) {
    a.x += 1;
    // Call f1 when g0 finishes.
    return g0(a.b)
        .add_continuation(
            MEFDN_MAKE_INLINED_FUNCTION(&f1)
        ,   a
        );
}

} // unnamed namespace

TEST(Deferred, Cont)
{
    A a;
    a.x = 0;
    a.b.x = 0;
    
    mefdn::deferred<void> d = f0(a);
    ASSERT_EQ(1, a.x);
    ASSERT_EQ(1, a.b.x);
    
    // No continuation is added to "d".
    mefdn::resumable res = d.set_terminal();
    ASSERT_FALSE(res.empty()); // Not finished yet
    
    ASSERT_EQ(1, a.x);
    ASSERT_EQ(1, a.b.x);
    
    ASSERT_FALSE(d.resume()); // Not finished yet
    
    ASSERT_EQ(11, a.x);
    ASSERT_EQ(12, a.b.x);
    
    ASSERT_TRUE(d.resume()); // Finished
    
    ASSERT_EQ(111, a.x);
    ASSERT_EQ(22, a.b.x);
}

namespace /*unnamed*/ {

mefdn::deferred<void> func(A& a)
{
    if (a.x < 10) {
        ++a.x;
        // Call func when the function is resumed.
        return mefdn::make_deferred(MEFDN_MAKE_INLINED_FUNCTION(&func), a);
    }
    else {
        // Immediately finish.
        return mefdn::make_ready_deferred();
    }
}

} // unnamed namespace

TEST(Deferred, Resume)
{
    A a;
    a.x = 0;
    
    mefdn::deferred<void> d = func(a);
    for (int i = 1; i < 10; i++) {
        ASSERT_EQ(i, a.x);
        d.resume();
    }
    
    ASSERT_EQ(10, a.x);
}

TEST(Deferred, Wait)
{
    A a;
    a.x = 0;
    
    mefdn::deferred<void> d = func(a);
    d.wait();
    
    ASSERT_EQ(10, a.x);
    
    d = mefdn::make_ready_deferred();
    d.wait();
}

