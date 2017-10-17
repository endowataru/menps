
#include "unittest.hpp"
#include <menps/mefdn/nonblocking/spsc_bounded_queue.hpp>
#include <menps/mefdn/thread.hpp>

namespace fdn = menps::mefdn;

TEST(SpscBoundedQueue, Serial)
{
    const fdn::uint64_t N = 10000;
    
    fdn::static_spsc_bounded_queue<fdn::uint64_t, 256> buf;
    
    fdn::uint64_t x = 0;
    
    for (fdn::uint64_t i = 1; i <= N; ++i)
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
    fdn::uint64_t n;
    
    void operator() ()
    {
        fdn::uint64_t x = 0;
        for (fdn::uint64_t i = 1; i <= n; ++i) {
            x += q.dequeue();
        }
        *result = x;
    }
};

} // unnamed namespace


TEST(SpscBoundedQueue, Spsc)
{
    typedef fdn::static_spsc_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
    const fdn::uint64_t N = 100000;
    
    fdn::uint64_t result = 0;
    queue_type buf;
    
    fdn::thread th{functor<queue_type>{&result, buf, N}};
    
    for (fdn::uint64_t i = 1; i <= N; ++i) {
        buf.enqueue(i);
    }
    
    th.join();
    
    ASSERT_EQ(N * (N+1) / 2, result);
}

