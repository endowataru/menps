
#pragma once

#include <mgbase/container/circular_buffer.hpp>
#include <mgbase/threading/spinlock.hpp>
#include <mgbase/threading/lock_guard.hpp>
#include <mgbase/scoped_ptr.hpp>
#include <mgbase/operation.hpp>
#include <mgcom/rma/pointer.hpp>

namespace mgcom {
namespace ibv {

class completer
{
    static const mgbase::size_t max_num_completions = 102400;
    
public:
    void initialize()
    {
        operations_ = new mgbase::operation[max_num_completions];
        dests_ = new rma::atomic_default_t*[max_num_completions];
        results_lptr_ = rma::allocate<rma::atomic_default_t>(max_num_completions);
        
        for (mgbase::uint64_t wr_id = 0; wr_id < max_num_completions; ++wr_id)
            queue_.push_back(wr_id);
    }
    
    void finalize()
    {
        operations_.reset();
        dests_.reset();
        mgcom::rma::deallocate(results_lptr_);
    }
    
    bool try_complete(const mgbase::operation& on_complete, mgbase::uint64_t* const wr_id_result)
    {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        if (!try_complete_nolock(on_complete, wr_id_result))
            return false;
        
        dests_[*wr_id_result] = MGBASE_NULLPTR;
        
        return true;
    }
    
    bool try_complete_with_result(
        const mgbase::operation&                            on_complete
    ,   mgbase::uint64_t* const                             wr_id_result
    ,   rma::atomic_default_t* const                        dest_ptr
    ,   mgcom::rma::local_ptr<rma::atomic_default_t>* const result_lptr_result
    ) {
        mgbase::lock_guard<mgbase::spinlock> lc(lock_);
        
        if (!try_complete_nolock(on_complete, wr_id_result))
            return false;
        
        dests_[*wr_id_result] = dest_ptr;
        *result_lptr_result = results_lptr_ + *wr_id_result;
        
        return true;
    }
    
private:
    bool try_complete_nolock(const mgbase::operation& on_complete, mgbase::uint64_t* const wr_id_result)
    {
        if (queue_.empty()) {
            return false;
        }
        
        const mgbase::uint64_t wr_id = queue_.front();
        queue_.pop_front();
        
        operations_[wr_id] = on_complete;
        
        *wr_id_result = wr_id;
        return true;
    }
    
public:
    void failed(const mgbase::uint64_t wr_id)
    {
        push_back(wr_id);
    }
    
    void notify(const mgbase::uint64_t wr_id)
    {
        if (dests_[wr_id] != MGBASE_NULLPTR) {
            // Copy the result to the specified buffer.
            *dests_[wr_id] = *(results_lptr_ + wr_id);
        }
        
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
    mgbase::scoped_ptr<rma::atomic_default_t* []> dests_;
    rma::local_ptr<rma::atomic_default_t> results_lptr_;
};

} // namespace ibv
} // namespace mgcom

