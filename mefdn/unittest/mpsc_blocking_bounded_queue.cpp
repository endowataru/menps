

#error "No longer maintained"

#include "unittest.hpp"
#include <mgbase/lockfree/mpsc_blocking_bounded_queue.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/threading/this_thread.hpp>

namespace /*unnamed*/ {

template <typename Queue>
struct functor
{
    Queue& q;
    mgbase::uint64_t n;
    
    void operator() ()
    {
        for (mgbase::uint64_t i = 1; i <= n; ++i) {
            while (!q.try_push(i)) {
                mgbase::this_thread::yield();
            }
            //q.notify();
        }
    }
};

} // unnamed namespace

TEST(MpscBlockingBoundedQueue, Serial)
{
    const mgbase::uint64_t N = 10000;
    
    mgbase::mpsc_blocking_bounded_queue<mgbase::uint64_t, 256> buf;
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        ASSERT_TRUE(buf.try_push(i));
        x += buf.dequeue();
    }
    
    ASSERT_EQ(N * (N+1) / 2, x);
}


TEST(MpscBlockingBoundedQueue, SpSc)
{
    typedef mgbase::mpsc_blocking_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
    const mgbase::uint64_t N = 100000;
    
    queue_type buf;
    
    mgbase::thread th{functor<queue_type>{buf, N}};
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        x += buf.dequeue();
    }
    
    th.join();
    
    ASSERT_EQ(N * (N+1) / 2, x);
}

TEST(MpscBlockingBoundedQueue, MpSc)
{
    const mgbase::uint64_t N = 10000;
    const mgbase::size_t num_threads = 10;
    
    typedef mgbase::mpsc_blocking_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
    queue_type buf;
    
    mgbase::scoped_ptr<mgbase::thread []> ths(new mgbase::thread[num_threads]);
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i] = mgbase::thread{functor<queue_type>{buf, N}};
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 0; i < num_threads * N; ++i) {
        x += buf.dequeue();
    }
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    //ASSERT_TRUE(buf.peek() == MGBASE_NULLPTR);
    //ASSERT_FALSE(buf.peek(&head));
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, x);
}


