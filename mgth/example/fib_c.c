
#include <mgth.h>
#include <stdio.h>
#include <inttypes.h>

typedef mgbase_uint64_t fib_int_t;

typedef struct fib_data
{
    fib_int_t * ret;
    fib_int_t   arg;
}
fib_data;

static void fib(void * const arg)
{
    const fib_data * d = (const fib_data *) arg;
    const fib_int_t n = d->arg;
    
    if (n == 0 || n == 1) {
        *d->ret = n;
        return;
    }
    
    const mgth_new_ult t =
        mgth_alloc_ult(MGBASE_ALIGNOF(fib_data), sizeof(fib_data));
   
    fib_int_t r1 = 0;
    fib_data * const d1 = (fib_data *) t.ptr;
    
    d1->ret = &r1;
    d1->arg = n-1;
    
    fib_int_t r2 = 0;
    fib_data d2 = { &r2, n-2 };
    
    mgth_fork(t, &fib);
    
    fib(&d2);
    
    mgth_join(t.id);
    
    *d->ret = r1 + r2;
}

int mgth_main(const int argc, char ** const argv)
{
    if (argc != 2) {
        fprintf(stderr, "%s [n]\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    const fib_int_t arg_n = (fib_int_t) atoi(argv[1]);
    
    fib_int_t result = 0;
    fib_data d = { &result, arg_n };
    
    fib(&d);
    
    printf("fib(%" PRIu64 ") = %" PRIu64 "\n", arg_n, result);
    
    return 0;
}

