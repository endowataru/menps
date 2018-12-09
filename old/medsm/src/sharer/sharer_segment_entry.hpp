
#pragma once

#include "basic_sharer_segment_entry.hpp"
#include "sharer_page.hpp"
#include "manager/manager_segment_proxy.hpp"

namespace menps {
namespace medsm {

class sharer_segment_entry;

struct sharer_segment_entry_traits
{
    typedef sharer_segment_entry    derived_type;
    
    typedef sharer_page             page_type;
    typedef page_id_t               page_id_type;
    
    typedef manager_segment_proxy       manager_type;
    typedef manager_segment_proxy_ptr   manager_ptr_type;
};

class sharer_segment_entry
    : public basic_sharer_segment_entry<sharer_segment_entry_traits>
{
    typedef basic_sharer_segment_entry<sharer_segment_entry_traits>   base;
    
    typedef mefdn::spinlock    lock_type;
    
public:
    typedef mefdn::unique_lock<lock_type>  unique_lock_type;
    
    template <typename Conf>
    explicit sharer_segment_entry(Conf&& conf)
        : base(mefdn::forward<Conf>(conf))
    { }
    
    unique_lock_type get_lock()
    {
        return unique_lock_type(lock_);
    }
    
private:
    lock_type lock_;
};

} // namespace medsm
} // namespace menps

