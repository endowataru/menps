
#pragma once

#include <mgbase/memory/mapped_memory.hpp>

namespace mgdsm {

class protector_space;

class protector_segment
{
public:
    template <typename Conf>
    explicit protector_segment(protector_space& sp, const Conf& conf);
    
    protector_space& get_space() const MGBASE_NOEXCEPT {
        return sp_;
    }
    
private:
    protector_space&        sp_;
    mgbase::mapped_memory   app_map_;
    mgbase::mapped_memory   sys_map_;
};

} // namespace mgdsm

