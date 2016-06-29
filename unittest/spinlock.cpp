
#include "unittest.hpp"
#include <mgbase/threading/thread.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/scoped_ptr.hpp>

namespace /*unnamed*/ {

int val;
mgbase::spinlock lock;

const mgbase::size_t count = 100000;

void f()
{
    for (int i = 0; i < count; ++i)
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock);
        ++val;
    }
}

}

TEST(Spinlock, Basic)
{
    val = 0;
    
    const mgbase::size_t num_threads = 5;
    
    mgbase::scoped_ptr<mgbase::thread []> ths(new mgbase::thread[num_threads]);
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i] = mgbase::thread(&f);
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    ASSERT_EQ(count * num_threads, val);
}

