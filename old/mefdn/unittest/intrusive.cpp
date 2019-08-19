
#include "unittest.hpp"
#include <menps/mefdn/container/intrusive_forward_list.hpp>

namespace fdn = menps::mefdn;

namespace /*unnamed*/ {

struct elem
    : fdn::intrusive_forward_list_node_base
{
    int x;
};

} // unnamed namespace

TEST(InstrusiveForwardList, Empty)
{
    fdn::intrusive_forward_list<elem> l;
    
    ASSERT_TRUE(l.empty());
    
}

TEST(InstrusiveForwardList, Basic)
{
    fdn::intrusive_forward_list<elem> l;
    
    elem e;
    e.x = 123;   
    l.push_front(e);
    
    ASSERT_FALSE(l.empty());
    
    int s = 0;
    for (auto&& a : l) {
        s += a.x;
    }
    
    ASSERT_EQ(123, s);
}

