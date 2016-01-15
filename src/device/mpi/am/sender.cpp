
#include "sender.ipp"

namespace mgcom {
namespace am {

namespace sender {

namespace /*unnamed*/ {

impl g_impl;

} // namespace unnamed

void initialize() {
    g_impl.initialize();
}
void finalize() {
    g_impl.finalize();
}

void add_ticket_to(process_id_t dest_proc, index_t ticket) {
    g_impl.add_ticket(dest_proc, ticket);
}

} // namespace sender

namespace untyped {

namespace detail {

mgbase::deferred<void> send_nb(send_cb& cb)
{
    return sender::impl::send_handlers<sender::g_impl>::start(cb);
}

} // namespace detail

} // namespace untyped

} // namespace am
} // namespace mgcom

