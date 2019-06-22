
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

//#include <menps/meult/sm.hpp>
#include <menps/meult/backend/mth/ult_policy.hpp>

#define USE_QDLOCK

#ifdef USE_QDLOCK
#include <menps/meult/qd/qdlock_mutex.hpp>
#endif

//namespace //my_ult = menps::meult::sm;
using my_ult = menps::meult::backend::mth::ult_policy;
#ifdef USE_QDLOCK
using mutex_t = menps::meult::qdlock_mutex<my_ult>;
#else
using mutex_t = my_ult::mutex;
#endif

#if 0
void lock_mtx(mutex_t* m)
{
    m->lock();
}
void unlock_mtx(mutex_t* m)
{
    m->unlock();
}
#endif

void f(long a, long b, long* r, mutex_t* mtx, long* p, long n_per_th)
{
    if (b - a == 1) {
        for (long i = 0; i < n_per_th; ++i) {
            mtx->lock();
            ++*p;
            mtx->unlock();
        }
        *r = a;
    }
    else {
        long c = (a + b) / 2;
        long r1 = 0, r2 = 0;
        my_ult::thread t(f, a, c, &r1, mtx, p, n_per_th);
        f(c, b, &r2, mtx, p, n_per_th);
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
        mutex_t mtx;
        long p = 0;
        my_ult::thread t(f, 0, nthreads, &r, &mtx, &p, n_per_th);
        t.join();
        double t1 = cur_time();
        double dt = t1 - t0;
        if (r == (nthreads - 1) * nthreads / 2 &&
            p == nthreads * n_per_th
        ) {
            printf("OK\n");
            printf("%ld thread creation/join in %.9f sec (%.3f per sec)\n",
                    nthreads, dt, nthreads / dt);
            printf("\%e sec per lock, ", dt / (nthreads * n_per_th));
            printf("\%e locks per sec\n", (nthreads * n_per_th) / dt);
        } else {
            printf("NG\n");
            return 1;
        }
    }
    return 0;
}
