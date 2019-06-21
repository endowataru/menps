
#include <cmpth/sct/def_sct_itf.hpp>

using ult_itf = cmpth::def_sct_itf;

#include "test.hpp"

int main(int argc, char* argv[])
{
    ult_itf::initializer sched_init;
    
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int res = context.run();
    return res;
}

