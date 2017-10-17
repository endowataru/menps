
#include "unittest.hpp"
#include <mgbase/nonblocking/spsc_bounded_queue.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/threading/this_thread.hpp>

TEST(SpscBoundedQueue, Serial)
{
    const mgbase::uint64_t N = 10000;
    
    mgbase::static_spsc_bounded_queue<mgbase::uint64_t, 256> buf;
    
    mgbase::uint64_t x = 0;
    
    for (mgbase::uint64_t i = 1; i <= N; ++i)
    {
        buf.enqueue(i);
        
        x += buf.dequeue();
    }
    
    ASSERT_EQ(N * (N+1) / 2, x);
}

namespace /*unnamed*/ {

template <typename Queue>
struct functor
{
    uint64_t* result;
    Queue& q;
    mgbase::uint64_t n;
    
    void operator() ()
    {
        mgbase::uint64_t x = 0;
        for (mgbase::uint64_t i = 1; i <= n; ++i) {
            x += q.dequeue();
        }
        *result = x;
    }
};

} // unnamed namespace


TEST(SpscBoundedQueue, Spsc)
{
    typedef mgbase::static_spsc_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
    const mgbase::uint64_t N = 100000;
    
    mgbase::uint64_t result = 0;
    queue_type buf;
    
    mgbase::thread th{functor<queue_type>{&result, buf, N}};
    
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        buf.enqueue(i);
    }
    
    th.join();
    
    ASSERT_EQ(N * (N+1) / 2, result);
}

