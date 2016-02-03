
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/logging/logger.hpp>

namespace mgcom {
namespace rma {

template <typename Request, index_t Size>
class request_queue
{
public:
    template <typename T>
    void initialize(const T& val)
    {
        finished_ = false;
        th_ = mgbase::thread(starter<T>(this, val));
        MGBASE_LOG_DEBUG("msg:Initialized RMA queue.");
    }
    
    void finalize()
    {
        finished_ = true;
        th_.join();
        MGBASE_LOG_DEBUG("msg:Finalized RMA queue.");
    }
    
    bool try_enqueue(const Request& req)
    {
        return buf_.try_push(req);
    }
    
private:
    template <typename Arg>
    class starter
    {
    public:
        starter(request_queue* self, const Arg& arg)
            : self_(self), arg_(arg) { }
        
        void operator() ()
        {
            while (!self_->finished_)
            {
                // Check the queue.
                if (Request* req = self_->buf_.peek()) {
                    // Call operator().
                    if (req->execute(arg_)) {
                        MGBASE_LOG_DEBUG("msg:Request succeeded.");
                        self_->buf_.pop();
                    }
                    else {
                        MGBASE_LOG_DEBUG("msg:Request failed. Postponed.");
                    }
                }
                // TODO: improve external interface
                arg_->poll();
            }
        }
        
    private:
        request_queue* self_;
        Arg arg_;
    };
    
    bool finished_;
    mgbase::thread th_;
    mgbase::mpsc_circular_buffer<Request, Size> buf_;
};

} // namespace rma
} // namespace mgcom

