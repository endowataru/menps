
#include <mgcom.hpp>

#include "am.ipp"
#include "sender_queue.hpp"

namespace mgcom {
namespace am {

#ifdef MGBASE_CPP11_SUPPORTED
namespace /*unnamed*/ {
#endif

impl g_impl;

#ifdef MGBASE_CPP11_SUPPORTED
} // unnamed namespace
#endif

void initialize()
{
    {
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        mpi_error::check(
            MPI_Comm_dup(MPI_COMM_WORLD, &get_comm())
        );
    }
    
    g_impl.initialize();
}

void finalize()
{
    g_impl.finalize();
    
    {
        mgbase::lock_guard<mpi_base::lock_type> lc(mpi_base::get_lock());
        mpi_error::check(
            MPI_Comm_free(&get_comm())
        );
    }
}

void poll() {
    g_impl.poll();
    sender_queue::poll();
}

namespace untyped {

void register_handler(
    handler_id_t       id
,   handler_callback_t callback
)
{
    g_impl.register_handler(id, callback);
}

namespace detail {

mgbase::deferred<void> send_nb(send_cb& cb)
{
    return impl::send_handlers<impl, g_impl>::start(cb);
}

} // namespace detail

} // namespace untyped

} // namespace am
} // namespace mgcom

