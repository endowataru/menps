
#include "unittest.hpp"

#include <mgbase/mutex.hpp>
#include <mgbase/condition_variable.hpp>
#include <mgbase/thread.hpp>

#include <queue>

namespace /*unnamed*/ {

std::queue<mgbase::int64_t> q;
mgbase::mutex mtx;
mgbase::condition_variable cv;
bool finished;
mgbase::int64_t result;

void f()
{
    mgbase::unique_lock<mgbase::mutex> lc(mtx);
    
    while (!finished || !q.empty())
    {
        while (!finished && q.empty())
            cv.wait(lc);
        
        if (!q.empty()) {
            result += q.front();
            q.pop();
        }
    }
}

} // unnamed namespace

TEST(ConditionVariable, Basic)
{
    finished = false;
    result = 0;
    
    mgbase::thread th(&f);
    
    const mgbase::int64_t N = 100000;
    
    for (mgbase::int64_t i = 1; i <= N; ++i) {
        mgbase::unique_lock<mgbase::mutex> lc(mtx);
        q.push(i);
        cv.notify_one();
    }
    
    {
        mgbase::unique_lock<mgbase::mutex> lc(mtx);
        finished = true;
        cv.notify_all();
    }
    
    th.join();
    
    ASSERT_EQ(N*(N+1)/2, result);
}

