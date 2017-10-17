
#include "unittest.hpp"
#include <menps/mefdn/nonblocking/mpsc_bounded_queue.hpp>
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/memory.hpp>

namespace fdn = menps::mefdn;

TEST(MpscBoundedQueue, Serial)
{
    const fdn::uint64_t N = 10000;
    
    fdn::static_mpsc_bounded_queue<fdn::uint64_t, 256> buf;
    
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
    Queue& q;
    fdn::uint64_t n;
    
    void operator() ()
    {
        for (fdn::uint64_t i = 1; i <= n; ++i) {
            q.enqueue(i);
        }
    }
};

} // unnamed namespace


TEST(MpscBoundedQueue, SpSc)
{
    typedef fdn::static_mpsc_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
    const fdn::uint64_t N = 100000;
    
    queue_type buf;
    
    fdn::thread th{functor<queue_type>{buf, N}};
    
    fdn::uint64_t x = 0;
    for (fdn::uint64_t i = 1; i <= N; ++i) {
        x += buf.dequeue();
    }
    
    th.join();
    
    ASSERT_EQ(N * (N+1) / 2, x);
}

TEST(MpscBoundedQueue, MpSc)
{
    const fdn::uint64_t N = 10000;
    const fdn::size_t num_threads = 10;
    
    typedef fdn::static_mpsc_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
    queue_type buf;
    
    const auto ths = fdn::make_unique<fdn::thread []>(num_threads);
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i] = fdn::thread{functor<queue_type>{buf, N}};
    
    fdn::uint64_t x = 0;
    for (fdn::uint64_t i = 0; i < num_threads * N; ++i) {
        x += buf.dequeue();
    }
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    const bool valid = buf.try_dequeue(1).valid();
    ASSERT_FALSE(valid);
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, x);
}

