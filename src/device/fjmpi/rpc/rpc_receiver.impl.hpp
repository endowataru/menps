
#pragma once

#include "rpc_receiver_thread.impl.hpp"

namespace mgcom {
namespace fjmpi {
namespace rpc {

namespace /*unnamed*/ {

class rpc_receiver
{
    static const index_t num_threads = 1;
    
public:
    void initialize(rpc_connection_pool& pool)
    {
        pool_ = &pool;
        
        invoker_.initialize();
        notifier_.initialize();
        
        ths_ = new rpc_receiver_thread[num_threads];
        
        for (index_t i = 0; i < num_threads; ++i)
            ths_[i].initialize(*pool_, notifier_, invoker_);
    }
    
    void finalize()
    {
        notifier_.set_finished();
        
        for (index_t i = 0; i < num_threads; ++i)
            ths_[i].finalize();
        
        ths_.reset();
        
        notifier_.finalize();
        
        invoker_.finalize();
    }
    
    void register_handler(const handler_id_t id, const handler_function_t callback)
    {
        invoker_.register_handler(id, callback);
    }
    
    void notify(
        const process_id_t  sender_proc
    ,   const int           local_nic
    ) {
        notifier_.notify(sender_proc, local_nic);
    }
    
private:
    rpc_connection_pool*                        pool_;
    rpc_notifier                                notifier_;
    rpc_invoker                                 invoker_;
    mgbase::scoped_ptr<rpc_receiver_thread []>  ths_;
};

} // unnamed namespace

} // namespace rpc
} // namespace fjmpi
} // namespace mgcom

