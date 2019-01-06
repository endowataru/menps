
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

//#include <menps/meult/sm.hpp>
#include <menps/meult/backend/mth/ult_policy.hpp>

//#define USE_QDLOCK

#ifdef USE_QDLOCK
#include <menps/meult/qd/qdlock_barrier.hpp>
#endif

//namespace //my_ult = menps::meult::sm;
using my_ult = menps::meult::backend::mth::ult_policy;
#ifdef USE_QDLOCK
using barrier_t = menps::meult::qdlock_barrier<my_ult>;
#else
using barrier_t = my_ult::barrier;
#endif

void f(long a, long b, long* r, barrier_t* bar, long n_per_th)
{
    if (b - a == 1) {
        for (long i = 0; i < n_per_th; ++i) {
            bar->arrive_and_wait();
        }
        *r = a;
    }
    else {
        long c = (a + b) / 2;
        long r1 = 0, r2 = 0;
        my_ult::thread t(f, a, c, &r1, bar, n_per_th);
        f(c, b, &r2, bar, n_per_th);
        t.join();
        *r = r1 + r2;
    }
}

double cur_time() {
  struct timeval tv[1];
  gettimeofday(tv, 0);
  return tv->tv_sec + tv->tv_usec * 1.0e-6;
}

int main(int argc, char ** argv)
{
    //my_ult::initializer init;

    long nthreads = (argc > 1 ? atol(argv[1]) : 50);
    long n_per_th = (argc > 1 ? atol(argv[2]) : 10000);
    long i;
    for (i = 0; i < 3; i++) {
        double t0 = cur_time();
        long r = 0;
        barrier_t bar(nthreads);
        long p = 0;
        my_ult::thread t(f, 0, nthreads, &r, &bar, n_per_th);
        t.join();
        double t1 = cur_time();
        double dt = t1 - t0;
        if (r == (nthreads - 1) * nthreads / 2) {
            printf("OK\n");
            printf("%ld thread creation/join in %.9f sec (%.3f per sec)\n",
                    nthreads, dt, nthreads / dt);
            printf("\%e sec per barrier, ", dt / n_per_th);
            printf("\%e barriers per sec\n", n_per_th / dt);
        } else {
            printf("NG\n");
            return 1;
        }
    }
    return 0;
}
