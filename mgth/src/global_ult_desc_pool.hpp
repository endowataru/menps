
#pragma once

#include <mgth/common.hpp>
#include "global_ult_ref.hpp"

#include <mgdsm/space_ref.hpp>

#include <mgbase/unique_ptr.hpp>
#include <mgbase/memory/allocatable.hpp>

namespace mgth {

class global_ult_desc_pool
{
public:
    static const mgbase::size_t stack_size = 64 << 10; // TODO: adjustable
    
    struct config
    {
        mgdsm::space_ref& space;
        void* stack_segment_ptr;
    };
    
    explicit global_ult_desc_pool(const config&);
    
    ~global_ult_desc_pool();
    
    global_ult_ref allocate_ult();
    
    void deallocate_ult(global_ult_ref&& th);
    
    global_ult_ref get_ult_ref_from_id(const ult_id& id);
    
private:
    class impl;
    mgbase::unique_ptr<impl> impl_;
};

#ifdef MGTH_ENABLE_ASYNC_WRITE_BACK
// TODO: move this definition
void global_ult_ref::do_update_stamp::operator() () const
{
    auto th = pool.get_ult_ref_from_id(this->id);
    
    auto lk = th.get_lock();
    
    const auto old_stamp = th.load_desc_member_relaxed(&global_ult_desc::old_stamp);
    const auto cur_stamp = th.load_desc_member_relaxed(&global_ult_desc::cur_stamp);
    
    MGBASE_LOG_INFO(
        "msg:Increment old stamp for thread.\t"
        "old_stamp:{}\t"
        "cur_stamp:{}\t"
        "assigned_stamp:{}\t"
        "{}"
    ,   old_stamp
    ,   cur_stamp
    ,   this->stamp
    ,   th.to_string()
    );
    
    MGBASE_ASSERT(cur_stamp - this->stamp >= 0);
    MGBASE_ASSERT(cur_stamp - old_stamp >= 0);
    
    MGBASE_ASSERT(old_stamp != this->stamp);
    if (old_stamp < this->stamp) {
        th.store_desc_member_relaxed(&global_ult_desc::old_stamp, this->stamp);
    }
    
    if (cur_stamp == this->stamp && (th.is_finished(lk) && th.is_detached(lk))) {
        lk.unlock();
        pool.deallocate_ult(mgbase::move(th));
    }
}

#endif

} // namespace mgth

