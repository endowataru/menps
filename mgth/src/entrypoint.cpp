
#include <mgth.hpp>
#include <mgas2.hpp>
#include <mgdsm.hpp>
#include "dist_scheduler.hpp"

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



#if 0

#include <mgas2.hpp>

#include "local_cache.hpp"
#include "coherence_manager.hpp"
#include "sigsegv_catcher.hpp"

#include <sstream>

namespace mgdsm {

namespace /*unnamed*/ {

mgbase::unique_ptr<local_cache> cache_;

mgbase::unique_ptr<coherence_manager> man_;

mgbase::unique_ptr<sigsegv_catcher> catcher_;

bool pass_segv(void* const ptr)
{
    if (cache_->is_in_range(ptr)) {
        man_->fetch(ptr);
        return true;
    }
    else
        return false;
}

} // unnamed namespace


namespace untyped {

void* allocate(std::size_t size_in_bytes, std::size_t page_size_in_bytes)
{
    return cache_->pointer();
}

} // namespace untyped


void initialize()
{
    const mgbase::size_t size_in_bytes = 128; // FIXME: make it adjustable
    
    const auto filename = fmt::format("mgdsm_cache_{}", mgcom::current_process_id());
    
    cache_.reset(new local_cache{filename.c_str(), size_in_bytes});
    
    man_.reset(new coherence_manager{*cache_});
    
    const sigsegv_catcher::config conf{
        &pass_segv
    /*,   cache_->pointer()
    ,   cache_->size_in_bytes()*/
    };
    
    catcher_.reset(new sigsegv_catcher{conf});
}

void finalize()
{
    catcher_.reset();
    
    man_.reset();
}

void reconcile()
{
    //man_->reconcile();
}

void flush()
{
    //man_->flush();
}

} // namespace mgdsm

int mgdsm_main(int argc, char* argv[]);

int main(int argc, char* argv[])
{
    // The communication interface is initialized here. (e.g. MPI_Init() is called.)
    mgcom::initialize(&argc, &argv);
    
    // Initialize PGAS.
    mgas2::initialize();
    
    mgdsm::initialize();
    
    int ret = mgdsm_main(argc, argv);
    
    mgdsm::finalize();
    
    // Finalize PGAS.
    mgas2::finalize();
    
    // Finalize the communication interface.
    mgcom::finalize();
    
    return ret;
}

#endif

