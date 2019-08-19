
#include "unittest.hpp"
#include <menps/mefdn/nonblocking/mpsc_locked_bounded_queue.hpp>
#include <menps/mefdn/thread.hpp>
#include <menps/mefdn/mutex.hpp>
#include <menps/mefdn/condition_variable.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>

namespace fdn = menps::mefdn;

TEST(MpscLockedBoundedQueue, Serial)
{
    const fdn::uint64_t N = 10000;
    
    fdn::static_mpsc_locked_bounded_queue<fdn::uint64_t, 256> buf;
    
    fdn::uint64_t x = 0;
    for (fdn::uint64_t i = 1; i <= N; ++i) {
        {
            auto t = buf.try_enqueue(1, true);
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
    const fdn::uint64_t N = 10000;
    
    fdn::static_mpsc_locked_bounded_queue<fdn::uint64_t, 256> buf;
    
    ASSERT_TRUE(buf.try_sleep());
    
    fdn::uint64_t x = 0;
    for (fdn::uint64_t i = 1; i <= N; ++i) {
        {
            auto t = buf.try_enqueue(1, true);
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
    fdn::uint64_t n;
    fdn::mutex& mtx;
    fdn::condition_variable& cv;
    
    void operator() ()
    {
        for (fdn::uint64_t i = 1; i <= n; ++i) {
            while (true) {
                auto t = q.try_enqueue(1, true); // CAS: tail = (tail + 0x2) & ~1;
                if (!t.valid() || t.size() == 0) {
                    fdn::this_thread::yield();
                    continue;
                }
                
                *t.begin() = i;
                t.commit(1); // entry.flag = true;
                
                if (t.is_sleeping()) { // (old_tail & 1) == 1
                    fdn::unique_lock<fdn::mutex> lk(mtx);
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
    typedef fdn::static_mpsc_locked_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
    const fdn::uint64_t N = 100000;
    
    queue_type buf;
    fdn::mutex mtx;
    fdn::condition_variable cv;
    
    fdn::thread th{functor<queue_type>{buf, N, mtx, cv}};
    
    fdn::uint64_t x = 0;
    {
        fdn::unique_lock<fdn::mutex> lk(mtx);
        for (fdn::uint64_t i = 1; i <= N; ++i) {
            while (true) {
                auto t = buf.try_dequeue(1); // check queue size & entry flag
                if (!t.valid() || t.size() == 0) { // (head&~1) == (tail&~1) ?
                    if (buf.try_sleep()) { // CAS: tail |= 1
                        cv.wait(lk);
                    }
                    else {
                        fdn::this_thread::yield();
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
    const fdn::uint64_t N = 10000;
    const fdn::size_t num_threads = 10;
    
    typedef fdn::static_mpsc_locked_bounded_queue<fdn::uint64_t, 256>  queue_type;
    
    queue_type buf;
    fdn::mutex mtx;
    fdn::condition_variable cv;
    
    const auto ths = fdn::make_unique<fdn::thread []>(num_threads);
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i] = fdn::thread{functor<queue_type>{buf, N, mtx, cv}};
    
    fdn::uint64_t x = 0;
    {
        fdn::unique_lock<fdn::mutex> lk(mtx);
        for (fdn::uint64_t i = 0; i < num_threads * N; ++i) {
            while (true) {
                auto t = buf.try_dequeue(1); // check queue size & entry flag
                if (!t.valid() || t.size() == 0) { // (head&~1) == (tail&~1) ?
                    if (buf.try_sleep()) { // CAS: tail |= 1
                        cv.wait(lk);
                    }
                    else {
                        fdn::this_thread::yield();
                    }
                    
                    continue;
                }
                
                x += *t.begin();
                
                t.commit(1); // reset entry flag; head += 0x2;
                
                break;
            }
        }
    }
    
    for (fdn::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, x);
}

