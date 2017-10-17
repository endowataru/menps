
#error "No longer maintained"

#include "unittest.hpp"

#include <mefdn/mutex.hpp>
#include <mefdn/condition_variable.hpp>
#include <mefdn/thread.hpp>

#include <queue>

namespace /*unnamed*/ {

std::queue<mefdn::int64_t> q;
mefdn::mutex mtx;
mefdn::condition_variable cv;
bool finished;
mefdn::int64_t result;

void f()
{
    mefdn::unique_lock<mefdn::mutex> lc(mtx);
    
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
    
    mefdn::thread th(&f);
    
    const mefdn::int64_t N = 100000;
    
    for (mefdn::int64_t i = 1; i <= N; ++i) {
        mefdn::unique_lock<mefdn::mutex> lc(mtx);
        q.push(i);
        cv.notify_one();
    }
    
    {
        mefdn::unique_lock<mefdn::mutex> lc(mtx);
        finished = true;
        cv.notify_all();
    }
    
    th.join();
    
    ASSERT_EQ(N*(N+1)/2, result);
}

