
#include <cmpth/wrap/mth_itf.hpp>

using ult_itf = cmpth::mth_itf;

#include "test.hpp"

int main(int argc, char* argv[])
{
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int res = context.run();
    return res;
}

