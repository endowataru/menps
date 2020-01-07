
#pragma once

#include <menps/medsm2/common.hpp>

namespace menps {
namespace medsm2 {

class svm_space_base
{
public:
    virtual ~svm_space_base() = default;

    virtual void* coll_alloc_seg(fdn::size_t seg_size, fdn::size_t blk_size) = 0;
    
    virtual void coll_alloc_global_var_seg(fdn::size_t seg_size, fdn::size_t blk_size, void* start_ptr) = 0;
    
    using mutex_id_t = mefdn::uint32_t;
    
    virtual mutex_id_t allocate_mutex() = 0;
    
    virtual void deallocate_mutex(mutex_id_t) = 0;
    
    virtual void lock_mutex(mutex_id_t) = 0;
    
    virtual void unlock_mutex(mutex_id_t) = 0;
    
    virtual bool compare_exchange_strong_acquire(
        mefdn::uint32_t&    target // TODO: define atomic type
    ,   mefdn::uint32_t&    expected
    ,   mefdn::uint32_t     desired
    ) = 0;
    
    virtual void store_release(mefdn::uint32_t*, mefdn::uint32_t) = 0;
    
    //mefdn::uint64_t load_acquire(mefdn::uint64_t*);
    
    virtual void barrier() = 0;
    
    virtual void pin(void*, fdn::size_t) = 0;
    
    virtual void unpin(void*, fdn::size_t) = 0;
    
    virtual void enable_on_this_thread() = 0;
    
    virtual void disable_on_this_thread() = 0;
    
    virtual void start_release_thread() = 0;
    
    virtual void stop_release_thread() = 0;
    
    virtual bool try_upgrade(void* ptr) = 0;

    virtual void enable_prof() noexcept = 0;
    virtual void disable_prof() noexcept = 0;
};

} // namespace medsm2
} // namespace menps

