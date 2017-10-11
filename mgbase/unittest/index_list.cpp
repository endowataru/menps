
#include "unittest.hpp"
#include <mgbase/container/index_list.hpp>

namespace /*unnamed*/ {



} // unnamed namespace

TEST(IndexList, Empty)
{
    mgbase::index_list<mgbase::size_t> l(100);
    
    ASSERT_TRUE(l.empty());
}

TEST(IndexList, Basic)
{
    const int N = 100;
    
    mgbase::index_list<int> l(N);
    
    for (int i = 0; i < N; ++i)
        l.push_front(i);
    
    ASSERT_FALSE(l.empty());
    
    int s = 0;
    MGBASE_RANGE_BASED_FOR(auto x, l) {
        s += x;
    }
    
    ASSERT_EQ(N * (N-1) / 2, s);
    
    for (int i = 0; i < N; ++i)
        l.pop_front();
    
    ASSERT_TRUE(l.empty());
}


