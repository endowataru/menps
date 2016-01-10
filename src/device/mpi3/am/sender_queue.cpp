
#include <mgcom.hpp>
#include <queue>

namespace mgcom {

namespace am {
namespace sender_queue {

namespace {

// TODO: naive implementation

class impl {
    struct entry {
        send_cb cb;
        mgbase::resumable res;
    };
    
public:
    void initialize() {
    
    }
    
    void finalize() {
    
    }
    
    void enqueue(
        handler_id_t    id
    ,   const void*     value
    ,   index_t         size
    ,   process_id_t    dest_proc
    ) {
        entries_.push(entry());
        entry& e = entries_.back();
        
        e.res = untyped::send_nb(e.cb, id, value, size, dest_proc)
            .set_terminal();
    }
    
    void poll() {
        if (entries_.empty())
            return;
        
        entry& e = entries_.front();
        if (e.res.checked_resume())
            entries_.pop();
    }
    
private:
    std::queue<entry> entries_;
};

impl g_impl;

} // namespace 

void initialize() {
    g_impl.initialize();
}

void finalize() {
    g_impl.finalize();
}

void enqueue(
    handler_id_t    id
,   const void*     value
,   index_t         size
,   process_id_t    dest_proc
)
{
    g_impl.enqueue(id, value, size, dest_proc);
}

void poll()
{
    g_impl.poll();
}

}

namespace untyped {

void reply(
    const callback_parameters* params
,   handler_id_t               id
,   const void*                value
,   index_t                    size
)
{
    sender_queue::enqueue(id, value, size, params->source);
}

} // namespace untyped

} // namespace am

}

