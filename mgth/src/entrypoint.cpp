
#include <mgth.hpp>
#include <mgas2.hpp>
#include <mgdsm.hpp>
#include "dist_scheduler.hpp"

//#include "disable_aslr.hpp"

namespace mgth {

namespace /*unnamed*/ {

mgdsm::dsm_interface_ptr g_dsm;

dist_scheduler_ptr g_sched;

int g_argc;
char** g_argv;
int g_ret;

void main_handler()
{
    // Call the user-defined main function.
    int ret = mgth_main(g_argc, g_argv);
}

inline thread_id_t to_thread_id(const mgult::ult_id& id) {
    return reinterpret_cast<thread_id_t>(id.ptr);
}
inline mgult::ult_id to_ult_id(const thread_id_t& id) {
    return { reinterpret_cast<void*>(id) };
}

} // unnamed namespace

thread_id_t fork(const fork_func_t func, void* const arg) {
    return to_thread_id(g_sched->fork(func, arg));
}

void* join(const thread_id_t id) {
    return g_sched->join(to_ult_id(id));
}

void detach(const thread_id_t id) {
    return g_sched->detach(to_ult_id(id));
}

void yield() {
    g_sched->yield();
}

MGBASE_NORETURN
void exit(void* const ret) {
    g_sched->exit(ret);
}

} // namespace mgth


int main(int argc, char* argv[])
{
    //mgth::disable_aslr(argc, argv);
    
    // The communication interface is initialized here. (e.g. MPI_Init() is called.)
    mgcom::initialize(&argc, &argv);
    
    mgth::g_argc = argc;
    mgth::g_argv = argv;
    
    // Initialize PGAS.
    mgas2::initialize();
    
    // Initialize DSM.
    mgth::g_dsm = mgdsm::make_auto_fetch_dsm();
    
    // Initialize a scheduler.
    mgth::g_sched = mgth::make_dist_scheduler(*mgth::g_dsm);
    
    // Start the scheduler loop.
    mgth::g_sched->loop(&mgth::main_handler);
    
    // Finalize the scheduler.
    mgth::g_sched.reset();
    
    // Finalize DSM.
    mgth::g_dsm.reset();
    
    // Finalize PGAS.
    mgas2::finalize();
    
    // Finalize the communication interface.
    mgcom::finalize();
    
    return mgth::g_ret;
}

