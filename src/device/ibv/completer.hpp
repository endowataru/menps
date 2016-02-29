
#pragma once

#include <mgbase/container/circular_buffer.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/operation.hpp>

namespace mgcom {

namespace ibv {

class completer
{
    static const mgbase::size_t max_num_completions = 102400;
    
public:
    void initialize()
    {
        operations_ = new mgbase::operation[max_num_completions];
        
        for (mgbase::uint64_t wr_id = 0; wr_id < max_num_completions; ++wr_id)
            queue_.push_back(wr_id);
    }
    
    void finalize()
    {
        operations_.reset();
    }
    
    bool try_complete(const mgbase::operation& on_complete, mgbase::uint64_t* const wr_id_result)
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        if (queue_.empty()) {
            return false;
        }
        
        const mgbase::uint64_t wr_id = queue_.front();
        queue_.pop_front();
        
        operations_[wr_id] = on_complete;
        
        *wr_id_result = wr_id;
        return true;
    }
    
    void failed(const mgbase::uint64_t wr_id)
    {
        push_back(wr_id);
    }
    
    void notify(const mgbase::uint64_t wr_id)
    {
        mgbase::execute(operations_[wr_id]);
        
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
    mgbase::scoped_ptr<mgbase::operation []> operations_;
};

} // namespace ibv

} // namespace mgcom

