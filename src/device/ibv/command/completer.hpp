
#pragma once

#include <mgbase/nonblocking/spsc_bounded_queue.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/callback.hpp>
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace ibv {

class completer
{
public:
    static const mgbase::size_t max_num_completions = 1 << 17;
    
private:
    typedef mgbase::uint64_t    wr_id_type;
    typedef mgbase::static_spsc_bounded_queue<wr_id_type, max_num_completions>    queue_type;
    
    typedef mgbase::callback<void ()>   callback_type;
    
public:
    completer()
        : cbs_(max_num_completions, callback_type{})
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
        
        MGBASE_DEFINE_DEFAULT_MOVE_NOEXCEPT_BASE_0(start_transaction, base)
        
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
        MGBASE_ASSERT(!cbs_[wr_id]);
        
        cbs_[wr_id] = cb;
    }
    
    void notify(const wr_id_type wr_id)
    {
        // Execute the callback.
        cbs_[wr_id]();
        
        cbs_[wr_id] = callback_type{};
        
        queue_.enqueue(wr_id);
    }

private:
    queue_type                  queue_;
    std::vector<callback_type>  cbs_;
};

} // namespace ibv
} // namespace mgcom

