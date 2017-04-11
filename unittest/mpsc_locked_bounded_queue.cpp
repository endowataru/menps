
#include "unittest.hpp"
#include <mgbase/nonblocking/mpsc_locked_bounded_queue.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/threading/this_thread.hpp>
#include <mgbase/mutex.hpp>
#include <mgbase/condition_variable.hpp>
#include <mgbase/unique_ptr.hpp>

TEST(MpscLockedBoundedQueue, Serial)
{
    const mgbase::uint64_t N = 10000;
    
    mgbase::static_mpsc_locked_bounded_queue<mgbase::uint64_t, 256> buf;
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        {
            auto t = buf.try_enqueue(1);
            ASSERT_TRUE(t.valid());
            *t.begin() = i;
            t.commit(1);
            
            ASSERT_FALSE(t.is_sleeping());
        }
        
        x += buf.dequeue();
    }
    
    ASSERT_EQ(N * (N+1) / 2, x);
}

TEST(MpscLockedBoundedQueue, Serial2)
{
    const mgbase::uint64_t N = 10000;
    
    mgbase::static_mpsc_locked_bounded_queue<mgbase::uint64_t, 256> buf;
    
    //buf.start_sleeping();
    ASSERT_TRUE(buf.try_sleep());
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        {
            auto t = buf.try_enqueue(1);
            ASSERT_TRUE(t.valid());
            *t.begin() = i;
            t.commit(1);
            
            ASSERT_EQ(i == 1, t.is_sleeping());
        }
        
        x += buf.dequeue();
    }
    
    ASSERT_EQ(N * (N+1) / 2, x);
}

namespace /*unnamed*/ {

template <typename Queue>
struct functor
{
    Queue& q;
    mgbase::uint64_t n;
    mgbase::mutex& mtx;
    mgbase::condition_variable& cv;
    
    void operator() ()
    {
        for (mgbase::uint64_t i = 1; i <= n; ++i) {
            while (true) {
                auto t = q.try_enqueue(1); // CAS: tail = (tail + 0x2) & ~1;
                if (!t.valid() || t.size() == 0) {
                    mgbase::this_thread::yield();
                    continue;
                }
                
                *t.begin() = i;
                t.commit(1); // entry.flag = true;
                
                if (t.is_sleeping()) { // (old_tail & 1) == 1
                    mgbase::unique_lock<mgbase::mutex> lk(mtx);
                    cv.notify_one();
                }
                
                break;
            }
        }
    }
};

} // unnamed namespace

TEST(MpscLockedBoundedQueue, SpSc)
{
    typedef mgbase::static_mpsc_locked_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
    const mgbase::uint64_t N = 100000;
    
    queue_type buf;
    mgbase::mutex mtx;
    mgbase::condition_variable cv;
    
    mgbase::thread th{functor<queue_type>{buf, N, mtx, cv}};
    
    mgbase::uint64_t x = 0;
    {
        mgbase::unique_lock<mgbase::mutex> lk(mtx);
        for (mgbase::uint64_t i = 1; i <= N; ++i) {
            while (true) {
                auto t = buf.try_dequeue(1); // check queue size & entry flag
                if (!t.valid() || t.size() == 0) { // (head&~1) == (tail&~1) ?
                    if (buf.try_sleep()) { // CAS: tail |= 1
                        cv.wait(lk);
                    }
                    else {
                        mgbase::this_thread::yield();
                    }
                    
                    continue;
                }
                
                x += *t.begin();
                
                t.commit(1); // reset entry flag; head += 0x2;
                
                break;
            }
        }
    }
    
    th.join();
    
    ASSERT_EQ(N * (N+1) / 2, x);
}

TEST(MpscLockedBoundedQueue, MpSc)
{
    const mgbase::uint64_t N = 10000;
    const mgbase::size_t num_threads = 10;
    
    typedef mgbase::static_mpsc_locked_bounded_queue<mgbase::uint64_t, 256>  queue_type;
    
    queue_type buf;
    mgbase::mutex mtx;
    mgbase::condition_variable cv;
    
    const auto ths = mgbase::make_unique<mgbase::thread []>(num_threads);
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i] = mgbase::thread{functor<queue_type>{buf, N, mtx, cv}};
    
    mgbase::uint64_t x = 0;
    {
        mgbase::unique_lock<mgbase::mutex> lk(mtx);
        for (mgbase::uint64_t i = 0; i < num_threads * N; ++i) {
            while (true) {
                auto t = buf.try_dequeue(1); // check queue size & entry flag
                if (!t.valid() || t.size() == 0) { // (head&~1) == (tail&~1) ?
                    if (buf.try_sleep()) { // CAS: tail |= 1
                        cv.wait(lk);
                    }
                    else {
                        mgbase::this_thread::yield();
                    }
                    
                    continue;
                }
                
                x += *t.begin();
                
                t.commit(1); // reset entry flag; head += 0x2;
                
                break;
            }
        }
    }
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    //ASSERT_TRUE(buf.peek() == MGBASE_NULLPTR);
    //ASSERT_FALSE(buf.peek(&head));
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, x);
}


