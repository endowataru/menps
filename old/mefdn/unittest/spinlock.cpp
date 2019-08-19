
#include "unittest.hpp"
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/thread/spinlock.hpp>
#include <menps/mefdn/thread/lock_guard.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace fdn = menps::mefdn;

namespace /*unnamed*/ {

int val;
fdn::spinlock lock;

const fdn::size_t count = 100000;

void f()
{
    for (fdn::size_t i = 0; i < count; ++i)
    {
        fdn::lock_guard<fdn::spinlock> lc(lock);
        ++val;
    }
}

}

TEST(Spinlock, Basic)
{
    val = 0;
    
    const fdn::size_t num_threads = 5;
    
    const auto ths = fdn::make_unique<fdn::thread []>(num_threads);
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i] = fdn::thread(&f);
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    ASSERT_EQ(count * num_threads, val);
}

