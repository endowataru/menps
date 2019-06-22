
#include "unittest.hpp"
#include <menps/meult/qd/qdlock_mutex.hpp>
#include <menps/meult/backend/mth.hpp>
#include <menps/mefdn/vector.hpp>

TEST(Qdlock, MutexCounter)
{
    using namespace menps;
    
    using menps::meult::backend::mth::ult_policy;
    
    using mutex_type =
        menps::meult::qdlock_mutex<ult_policy>;
    
    const int nthreads = 10;
    const int count_per_thread = 10000;
    
    mutex_type mtx;
    int count = 0;
    
    mefdn::vector<ult_policy::thread> ths;
    for (int i = 0; i < nthreads; ++i) {
        ths.emplace_back([&, i] {
            for (int j = 0; j < count_per_thread; ++j) {
                //fmt::print("start lock: {}\n", i);
                mtx.lock();
                //fmt::print("{} {}\n", i, count);
                ++count;
                mtx.unlock();
            }
        });
    }
    
    for (auto& th : ths) {
        th.join();
    }
    
    ASSERT_EQ(nthreads * count_per_thread, count);
}


