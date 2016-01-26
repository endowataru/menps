
#include "unittest.hpp"
#include <mgbase/threading/thread.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>

namespace /*unnamed*/ {

int val;
mgbase::spinlock lock;

void f()
{
    for (int i = 0; i < 1000000; ++i)
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock);
        ++val;
    }
}

}

TEST(Spinlock, Basic)
{
    val = 0;
    
    std::vector<mgbase::thread> ths;
    for (int i = 0; i < 10; ++i)
        ths.push_back(mgbase::thread(&f));
    
    for (std::vector<mgbase::thread>::iterator itr = ths.begin(); itr != ths.end(); ++itr)
        itr->join();
    
    ASSERT_EQ(10000000, val);
}

