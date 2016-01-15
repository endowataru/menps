
#include "receiver.ipp"

namespace mgcom {
namespace am {

namespace receiver {

const index_t constants::max_num_tickets;

namespace /*unnamed*/ {

impl g_impl;

} // unnamed namespace

void initialize() {
    g_impl.initialize();
}

void finalize() {
    g_impl.finalize();
}

void poll() {
    g_impl.poll();
}

index_t pull_tickets_from(process_id_t src_proc) {
    return g_impl.pull_tickets_from(src_proc);
}

} // namespace receiver

namespace untyped {

void register_handler(
    handler_id_t       id
,   handler_callback_t callback
)
{
    receiver::g_impl.register_handler(id, callback);
}

} // namespace untyped

} // namespace am
} // namespace mgcom

