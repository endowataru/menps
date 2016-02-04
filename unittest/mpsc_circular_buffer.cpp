
#include "unittest.hpp"
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/threading/thread.hpp>

namespace /*unnamed*/ {

mgbase::uint64_t N = 100000;
mgbase::mpsc_circular_buffer<int, 256> buf;

inline void f()
{
    for (mgbase::uint64_t i = 1; i <= N; ++i) {
        while (!buf.try_push(i))
        { }
    }
}

} // unnamed namespace

TEST(MpscCircularBuffer, Concurrent)
{
    std::vector<mgbase::thread> ths;
    
    for (mgbase::size_t i = 0; i < 10; ++i) {
        ths.push_back(
            mgbase::thread(&f)
        );
    }
    
    mgbase::uint64_t x = 0;
    for (mgbase::uint64_t i = 0; i < ths.size() * N; ++i) {
        int* elem;
        while ((elem = buf.peek()) == MGBASE_NULLPTR) { }
        x += *elem;
        buf.pop();
    }
    
    for (std::vector<mgbase::thread>::iterator itr = ths.begin(); itr != ths.end(); ++itr)
    {
        itr->join();
    }
    
    ASSERT_TRUE(buf.peek() == MGBASE_NULLPTR);
    //ASSERT_FALSE(buf.peek(&head));
    
    ASSERT_EQ(ths.size() * N * (N+1) / 2, x);
}


