
#include <cmpth/wrap/ctmth_itf.hpp>

using ult_itf = cmpth::ctmth_itf;

#include "test.hpp"

int main(int argc, char* argv[])
{
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int res = context.run();
    return res;
}

