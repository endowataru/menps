
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include <mgth.hpp>

void f(long a, long b, long* r)
{
    if (b - a == 1) {
        *r = a;
    }
    else {
        long c = (a + b) / 2;
        long r1 = 0, r2 = 0;
        mgth::ult::thread t(f, a, c, &r1);
        f(c, b, &r2);
        t.join();
        *r = r1 + r2;
    }
}

double cur_time() {
  struct timeval tv[1];
  gettimeofday(tv, 0);
  return tv->tv_sec + tv->tv_usec * 1.0e-6;
}

int mgth_main(int argc, char ** argv)
{
    long nthreads = (argc > 1 ? atol(argv[1]) : 100000);
    long i;
    for (i = 0; i < 3; i++) {
        double t0 = cur_time();
        long r = 0;
        mgth::ult::thread t(f, 0, nthreads, &r);
        t.join();
        double t1 = cur_time();
        double dt = t1 - t0;
        if (r == (nthreads - 1) * nthreads / 2) {
            printf("OK\n");
            printf("%ld thread creation/join in %.9f sec (%.3f per sec)\n",
                    nthreads, dt, nthreads / dt);
        } else {
            printf("NG\n");
            return 1;
        }
    }
    return 0;
}
