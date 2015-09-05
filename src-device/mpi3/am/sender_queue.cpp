
#include <mgcom.hpp>
#include <queue>

namespace mgcom {

namespace am {
namespace sender_queue {

namespace {

// TODO: naive implementation

class impl {
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
        cbs_.push(send_cb());
        send_cb& cb = cbs_.back();
        send_nb(&cb, id, value, size, dest_proc);
    }
    
    void poll() {
        if (cbs_.empty())
            return;
        
        send_cb& cb = cbs_.front();
        if (mgbase::control::proceed(cb))
            cbs_.pop();
    }
    
private:
    std::queue<send_cb> cbs_;
};

impl g_impl;

}

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

void reply(
    const callback_parameters* params
,   handler_id_t               id
,   const void*                value
,   index_t                    size
)
{
    sender_queue::enqueue(id, value, size, params->source);
}

}

}

