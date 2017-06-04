
#pragma once

#include "basic_sharer_space.hpp"
#include "sharer_segment.hpp"
#include "manager/manager_space.hpp"
#include "segment_locator.hpp"
#include <mgbase/unique_ptr.hpp>
#include <mgdsm/ult.hpp>

namespace mgdsm {

class sharer_space;

struct sharer_space_policy
{
    typedef sharer_space                        derived_type;
    
    typedef sharer_segment                      segment_type;
    typedef mgbase::unique_ptr<segment_type>    segment_ptr_type;
    typedef segment_id_t                        segment_id_type;
    typedef sharer_segment::accessor            segment_accessor_type;
    typedef manager_space::segment_conf         segment_conf_type;
    
    typedef ult::mutex                          lock_type;
    typedef ult::unique_lock<lock_type>         unique_lock_type;
    
    typedef manager_segment_proxy_ptr           manager_segment_ptr_type;
    
    static unique_lock_type get_lock(lock_type& lock) {
        return unique_lock_type(lock);
    }
};

class sharer_space
    : public basic_sharer_space<sharer_space_policy>
{
    typedef basic_sharer_space<sharer_space_policy> base;
    
    typedef sharer_space_policy::segment_ptr_type   segment_ptr_type;
    
public:
    template <typename Conf>
    explicit sharer_space(const Conf& conf)
        : base(conf)
        , manager_(MGBASE_NULLPTR)
    { }
    
    class proxy;
    
    inline proxy make_proxy_collective();
    
    void set_manager(manager_space& manager)
    {
        MGBASE_ASSERT(this->manager_ == MGBASE_NULLPTR);
        this->manager_ = &manager;
    }
    
    inline sharer_segment::accessor get_segment_accessor(segment_id_t);
    
private:
    friend class basic_sharer_space<sharer_space_policy>;
    // friend base;
    
    manager_space& get_manager() {
        MGBASE_ASSERT(this->manager_ != MGBASE_NULLPTR);
        return *this->manager_;
    }
    
    manager_space*  manager_;
};

} // namespace mgdsm

