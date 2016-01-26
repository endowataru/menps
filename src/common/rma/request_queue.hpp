
#pragma once

#include <mgcom/common.hpp>
#include <mgbase/lockfree/mpsc_circular_buffer.hpp>
#include <mgbase/threading/thread.hpp>
#include <mgbase/logging/logger.hpp>
#include <queue> // TODO

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
                mgbase::size_t head;
                if (Request* req = self_->buf_.peek()) {
                    // Call operator().
                    if ((*req)(arg_)) {
                        MGBASE_LOG_DEBUG("msg:Request succeeded.");
                        self_->buf_.pop();
                    }
                    else {
                        // Postpone the request.
                        //self_->reordered_.push(*req);
                        
                        MGBASE_LOG_DEBUG("msg:Request failed. Postponed.");
                    }
                    //self_->buf_.pop();
                }
                /*else {
                    if (!self_->reordered_.empty())
                    {
                        MGBASE_LOG_DEBUG("msg:Found reordered request.");
                        
                        Request re_req = self_->reordered_.front();
                        self_->reordered_.pop();
                        
                        if (re_req(arg_)) {
                            MGBASE_LOG_DEBUG("msg:Reordered request succeeded.");
                        }
                        else {
                            self_->reordered_.push(re_req);
                            MGBASE_LOG_DEBUG("msg:Reordered request failed. Pushed to reordered queue again.");
                        }
                    }
                }*/
                // TODO: improve external interface
                arg_->poll();
                //}
                
                /*// Check the queue.
                if (self_->try_dequeue(&req))
                {
                    // Call operator().
                    if (req(arg_)) {
                        MGBASE_LOG_DEBUG("msg:Request succeeded.");
                    }
                    else {
                        // Postpone the request.
                        self_->reordered_.push(req);
                        
                        MGBASE_LOG_DEBUG("msg:Request failed. Pushed to reordered queue.");
                    }
                }
                else {
                    if (!self_->reordered_.empty())
                    {
                        MGBASE_LOG_DEBUG("msg:Found reordered request.");
                        
                        req = self_->reordered_.front();
                        self_->reordered_.pop();
                        
                        if (req(arg_)) {
                            MGBASE_LOG_DEBUG("msg:Reordered request succeeded.");
                        }
                        else {
                            self_->reordered_.push(req);
                            MGBASE_LOG_DEBUG("msg:Reordered request failed. Pushed to reordered queue again.");
                        }
                    }
                    
                    MGBASE_LOG_VERBOSE("msg:Poll.");
                    
                    // TODO: improve external interface
                    arg_->poll();
                }*/
            #if 0
                Request req;
                
                // Check the queue.
                if (self_->try_dequeue(&req))
                {
                    // Call operator().
                    if (req(arg_)) {
                        MGBASE_LOG_DEBUG("msg:Request succeeded.");
                    }
                    else {
                        // Postpone the request.
                        self_->reordered_.push(req);
                        
                        MGBASE_LOG_DEBUG("msg:Request failed. Pushed to reordered queue.");
                    }
                }
                else {
                    if (!self_->reordered_.empty())
                    {
                        MGBASE_LOG_DEBUG("msg:Found reordered request.");
                        
                        req = self_->reordered_.front();
                        self_->reordered_.pop();
                        
                        if (req(arg_)) {
                            MGBASE_LOG_DEBUG("msg:Reordered request succeeded.");
                        }
                        else {
                            self_->reordered_.push(req);
                            MGBASE_LOG_DEBUG("msg:Reordered request failed. Pushed to reordered queue again.");
                        }
                    }
                    
                    MGBASE_LOG_VERBOSE("msg:Poll.");
                    
                    // TODO: improve external interface
                    arg_->poll();
                }
            #endif
            }
        }
        
    private:
        request_queue* self_;
        Arg arg_;
    };
    
    bool finished_;
    mgbase::thread th_;
    mgbase::mpsc_circular_buffer<Request, Size> buf_;
    // TODO: avoid dynamic allocation
    std::queue<Request> reordered_;
};

} // namespace rma
} // namespace mgcom


