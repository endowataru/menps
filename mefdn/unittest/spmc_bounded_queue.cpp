
#include "unittest.hpp"
#include <menps/mefdn/nonblocking/spmc_bounded_queue.hpp>
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/memory.hpp>

namespace fdn = menps::mefdn;

TEST(SpmcBoundedQueue, Serial)
{
    const fdn::uint64_t N = 10000;
    
    fdn::static_spmc_bounded_queue<fdn::uint64_t, 256> buf;
    
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


TEST(SpmcBoundedQueue, Spsc)
{
    typedef fdn::static_spmc_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
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

TEST(SpmcBoundedQueue, Spmc)
{
    const fdn::uint64_t N = 10000;
    const fdn::size_t num_threads = 10;
    
    typedef fdn::static_spmc_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
    std::vector<fdn::uint64_t> results(num_threads, 0);
    
    queue_type buf;
    
    const auto ths = fdn::make_unique<fdn::thread []>(num_threads);
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i] = fdn::thread{functor<queue_type>{&results[i], buf, N}};
    
    for (fdn::uint64_t i = 0; i < num_threads; ++i) {
        for (fdn::uint64_t j = 1; j <= N; ++j) {
            buf.enqueue(j);
        }
    }
    
    fdn::uint64_t sum = 0;
    for (fdn::size_t i = 0; i < num_threads; ++i) {
        ths[i].join();
        sum += results[i];
    }
    
    const bool valid = buf.try_dequeue(1).valid();
    ASSERT_FALSE(valid);
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, sum);
}

