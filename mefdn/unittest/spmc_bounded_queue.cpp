
#include "unittest.hpp"
#include <mgbase/nonblocking/spmc_bounded_queue.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/threading/this_thread.hpp>

TEST(SpmcBoundedQueue, Serial)
{
    const mgbase::uint64_t N = 10000;
    
    mgbase::static_spmc_bounded_queue<mgbase::uint64_t, 256> buf;
    
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


TEST(SpmcBoundedQueue, Spsc)
{
    typedef mgbase::static_spmc_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
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

TEST(SpmcBoundedQueue, Spmc)
{
    const mgbase::uint64_t N = 10000;
    const mgbase::size_t num_threads = 10;
    
    typedef mgbase::static_spmc_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
    std::vector<mgbase::uint64_t> results(num_threads, 0);
    
    queue_type buf;
    
    mgbase::scoped_ptr<mgbase::thread []> ths(new mgbase::thread[num_threads]);
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i] = mgbase::thread{functor<queue_type>{&results[i], buf, N}};
    
    for (mgbase::uint64_t i = 0; i < num_threads; ++i) {
        for (mgbase::uint64_t j = 1; j <= N; ++j) {
            buf.enqueue(j);
        }
    }
    
    mgbase::uint64_t sum = 0;
    for (mgbase::size_t i = 0; i < num_threads; ++i) {
        ths[i].join();
        sum += results[i];
    }
    
    const bool valid = buf.try_dequeue(1).valid();
    ASSERT_FALSE(valid);
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, sum);
}

