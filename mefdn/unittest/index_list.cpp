
#include "unittest.hpp"
#include <menps/mefdn/container/index_list.hpp>

namespace fdn = menps::mefdn;

TEST(IndexList, Empty)
{
    fdn::index_list<fdn::size_t> l(100);
    
    ASSERT_TRUE(l.empty());
}

TEST(IndexList, Basic)
{
    const int N = 100;
    
    fdn::index_list<int> l(N);
    
    for (int i = 0; i < N; ++i)
        l.push_front(i);
    
    ASSERT_FALSE(l.empty());
    
    int s = 0;
    for (auto x : l) {
        s += x;
    }
    
    ASSERT_EQ(N * (N-1) / 2, s);
    
    for (int i = 0; i < N; ++i)
        l.pop_front();
    
    ASSERT_TRUE(l.empty());
}


