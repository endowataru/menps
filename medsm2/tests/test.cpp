
#include <menps/medsm2/itf/dsm_itf.hpp>
#include <menps/medsm2/svm/mpi_svm_space.hpp>
#include <menps/mefdn/disable_aslr.hpp>

using menps::medsm2::dsm_itf;

#include "test.hpp"

extern "C" {

extern void* _dsm_data_begin;
extern void* _dsm_data_end;

} // extern "C"

int main(int argc, char* argv[])
{
    doctest::Context context;
    context.applyCommandLine(argc, argv);
    
    dsm_itf::dsm_facade_type df{&argc, &argv};
    df.set_svm_space(menps::medsm2::make_mpi_svm_space(df.get_com_itf()));
    
    dsm_itf::set_dsm_facade(df);
    
    //df.init_global_var_seg(&_dsm_data_begin, &_dsm_data_end, MEDSM2_GLOBAL_VAR_BLOCK_SIZE);
    
    df.init_heap_seg(1024*4096, 4096); // FIXME
    
    df.enable_on_this_thread();
    int res = context.run();
    
    df.disable_on_this_thread();
    
    return res;
}

