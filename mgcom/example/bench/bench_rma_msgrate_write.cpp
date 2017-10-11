
#include "bench_rma_msgrate_main.hpp"

int main(int argc, char* argv[])
{
    return bench_main<bench_rma_msgrate<true>>(argc, argv, "write");
}

