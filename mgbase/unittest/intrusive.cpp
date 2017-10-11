
#include "unittest.hpp"
#include <mgbase/container/intrusive_forward_list.hpp>

namespace /*unnamed*/ {

struct elem
    : mgbase::intrusive_forward_list_node_base
{
    int x;
};

} // unnamed namespace

TEST(InstrusiveForwardList, Empty)
{
    mgbase::intrusive_forward_list<elem> l;
    
    ASSERT_TRUE(l.empty());
    
}

TEST(InstrusiveForwardList, Basic)
{
    mgbase::intrusive_forward_list<elem> l;
    
    elem e;
    e.x = 123;   
    l.push_front(e);
    
    ASSERT_FALSE(l.empty());
    
    int s = 0;
    MGBASE_RANGE_BASED_FOR(auto&& a, l) {
        s += a.x;
    }
    
    ASSERT_EQ(123, s);
}

