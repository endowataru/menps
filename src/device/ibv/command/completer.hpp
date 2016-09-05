
#pragma once

#include <mgbase/nonblocking/spsc_bounded_queue.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/callback.hpp>
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace ibv {

class completer
{
public:
    static const mgbase::size_t max_num_completions = 102400;
    
private:
    typedef mgbase::uint64_t    wr_id_type;
    typedef mgbase::static_spsc_bounded_queue<wr_id_type, max_num_completions>    queue_type;
    
    typedef mgbase::callback<void ()>   callback_type;
    
public:
    completer()
        : cbs_(max_num_completions)
    {
        // TODO: Exactly insert max_num_completions elements
        for (wr_id_type wr_id = 0; wr_id < max_num_completions - 1; ++wr_id)
            queue_.enqueue(wr_id);
    }
    
    ~completer() = default;
    
    completer(const completer&) = delete;
    completer& operator = (const completer&) = delete;
    
    class start_transaction
        : public queue_type::dequeue_transaction
    {
        typedef queue_type::dequeue_transaction base;
        
    public:
        explicit start_transaction(base&& t)
            : base(mgbase::move(t))
            { }
        
        
        start_transaction(const start_transaction&) = delete;
        start_transaction& operator = (const start_transaction&) = delete;
        
        #ifdef MGBASE_CXX11_MOVE_CONSTRUCTOR_DEFAULT_SUPPORTED
        start_transaction(start_transaction&&) = default;
        #else
        start_transaction(start_transaction&& other)
            : base(mgbase::move(other)) { }
        #endif
        
        #if 0
        #ifdef MGBASE_CXX11_MOVE_ASSIGNMENT_DEFAULT_SUPPORTED
        start_transaction& operator = (start_transaction&&) = default;
        #else
        start_transaction& operator = (start_transaction&& other) {
            static_cast<base&>(*this) = mgbase::move(other);
            return *this;
        }
        #endif
        #endif
        
        void commit()
        {
            base::commit(1);
        }
        
        void rollback()
        {
            base::commit(0);
        }
    };
    
    start_transaction try_start()
    {
        auto t = queue_.try_dequeue(1);
        
        return start_transaction(mgbase::move(t));
    }
    
    void set_on_complete(const wr_id_type wr_id, const callback_type& cb)
    {
        cbs_[wr_id] = cb;
    }
    
    void notify(const wr_id_type wr_id)
    {
        // Execute the callback.
        cbs_[wr_id]();
        
        queue_.enqueue(wr_id);
    }

private:
    queue_type queue_;
    
    #if 0
    bool try_complete(mgbase::uint64_t* const wr_id_result)
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        if (queue_.empty()) {
            return false;
        }
        
        const mgbase::uint64_t wr_id = queue_.front();
        queue_.pop_front();
        
        *wr_id_result = wr_id;
        return true;
    }
    
    void set_on_complete(const mgbase::uint64_t wr_id, const mgbase::callback<void ()>& on_complete)
    {
        cbs_[wr_id] = on_complete;
    }
    
    void failed(const mgbase::uint64_t wr_id)
    {
        push_back(wr_id);
    }
    
    void notify(const mgbase::uint64_t wr_id)
    {
        // Execute the callback.
        cbs_[wr_id]();
        
        push_back(wr_id);
    }
    
private:
    void push_back(const mgbase::uint64_t wr_id)
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        queue_.push_back(wr_id);
    }
    
    mgbase::spinlock lock_;
    mgbase::static_circular_buffer<mgbase::uint64_t, max_num_completions> queue_;
    #endif
    std::vector<callback_type> cbs_;
};

} // namespace ibv
} // namespace mgcom

