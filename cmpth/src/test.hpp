
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include <vector>

inline void assign_int(int* p, int x) {
    *p = x;
}

TEST_CASE("A thread is created and joined") {
    int ret = 0;
    ult_itf::thread t{&assign_int, &ret, 123};
    t.join();
    REQUIRE(ret == 123);
}

TEST_CASE("Many threads are created and joined") {
    const int num_ths = 1000;
    std::vector<ult_itf::thread> ths;
    std::vector<int> rets(1000, 0);
    for (int i = 0; i < num_ths; ++i) {
        ths.emplace_back(assign_int, &rets[i], i*10);
    }
    for (int i = 0; i < num_ths; ++i) {
        ths[i].join();
        REQUIRE(rets[i] == i*10);
    }
}


using fib_int_t = cmpth::fdn::uint64_t;

inline void fib_rec(fib_int_t* ret, const fib_int_t n) {
    if (n == 0 || n == 1) {
        *ret = n;
        return;
    }
    
    fib_int_t r1 = 0;
    fib_int_t r2 = 0;
    
    ult_itf::thread th{ &fib_rec, &r1, n-1 };
    fib_rec(&r2, n-2);
    th.join();
    
    *ret = r1 + r2;
}

TEST_CASE("Calculate fib(30) in parallel") {
    fib_int_t ret = 0;
    fib_rec(&ret, 30);
    REQUIRE(ret == 832040);
}
TEST_CASE("Calculate fib(35) in parallel") {
    fib_int_t ret = 0;
    fib_rec(&ret, 35);
    REQUIRE(ret == 9227465);
}


struct fib_arg {
    fib_int_t*  ret;
    fib_int_t   n;
};

void fib_ptr_rec(void* const arg)
{
    const auto& d = *static_cast<const fib_arg*>(arg);
    const auto ret = d.ret;
    const auto n = d.n;
    
    if (n == 0 || n == 1) {
        *ret = n;
        return;
    }
    
    fib_int_t r1 = 0;
    fib_int_t r2 = 0;
    
    fib_arg arg1{ &r1, n-1 };
    fib_arg arg2{ &r2, n-2 };
    
    ult_itf::thread th{ &fib_ptr_rec, &arg1 };
    fib_ptr_rec(&arg2);
    th.join();
    
    *ret = r1 + r2;
}

TEST_CASE("Calculate fib(30) in parallel using pointer interface") {
    fib_int_t ret = 0;
    fib_arg arg{ &ret, 30 };
    fib_ptr_rec(&arg);
    REQUIRE(ret == 832040);
}
TEST_CASE("Calculate fib(35) in parallel using pointer interface") {
    fib_int_t ret = 0;
    fib_arg arg{ &ret, 35 };
    fib_ptr_rec(&arg);
    REQUIRE(ret == 9227465);
}


inline void do_lock(ult_itf::mutex* m, int* out, int n) {
    for (int i = 0; i < n; ++i) {
        m->lock();
        ++*out;
        m->unlock();
    }
}

TEST_CASE("Execute many locks on mutex") {
    const int num_ths = 1000;
    const int n_per_th = 10000;
    std::vector<ult_itf::thread> ths;
    ult_itf::mutex mtx;
    int ret = 0;
    for (int i = 0; i < num_ths; ++i) {
        ths.emplace_back(do_lock, &mtx, &ret, n_per_th);
    }
    for (auto& th : ths) {
        th.join();
    }
    REQUIRE(ret == num_ths*n_per_th);
}


