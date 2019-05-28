
#include <cmpth/smth/default_smth.hpp>

using ult_itf = cmpth::default_smth_itf;

#include "test.hpp"

int main(int argc, char* argv[])
{
    cmpth::smth_scheduler<cmpth::default_smth_policy> sched;
    
    const char* const num_wks_str = std::getenv("CMPTH_NUM_WORKERS");
    int num_wks = num_wks_str ? std::atoi(num_wks_str) : 1;
    sched.make_workers(num_wks);
    
    cmpth::smth_scheduler<cmpth::default_smth_policy>::initializer init{sched};
    
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    int res = context.run();
    return res;
}

