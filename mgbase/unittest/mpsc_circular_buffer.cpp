
#include "unittest.hpp"
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/threading/this_thread.hpp>

namespace /*unnamed*/ {

mgbase::uint64_t N = 2000;
mgbase::mpsc_circular_buffer<mgbase::uint64_t, 256> buf;

inline void f()
{
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        while (!buf.try_push(i)) {
            mgbase::this_thread::yield();
        }
    }
}

} // unnamed namespace

TEST(MpscCircularBuffer, Concurrent)
{
    const mgbase::size_t num_threads = 10;
    mgbase::scoped_ptr<mgbase::thread []> ths(new mgbase::thread[num_threads]);
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i] = mgbase::thread(&f);
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 0; i < num_threads * N; ++i) {
        mgbase::uint64_t* elem;
        while ((elem = buf.peek()) == MGBASE_NULLPTR) { }
        x += *elem;
        buf.pop();
    }
    
    for (mgbase::size_t i = 0; i < num_threads; ++i)
        ths[i].join();
    
    ASSERT_TRUE(buf.peek() == MGBASE_NULLPTR);
    //ASSERT_FALSE(buf.peek(&head));
    
    ASSERT_EQ(num_threads * N * (N+1) / 2, x);
}


