
#pragma once

#include <menps/mefdn/memory/mapped_memory.hpp>

namespace menps {
namespace medsm {

class protector_space;

class protector_segment
{
public:
    template <typename Conf>
    explicit protector_segment(protector_space& sp, const Conf& conf);
    
    protector_space& get_space() const noexcept {
        return sp_;
    }
    
    void* get_app_ptr() const noexcept
    {
        return app_map_.get();
    }
    
private:
    protector_space&        sp_;
    mefdn::mapped_memory   sys_map_;
    mefdn::mapped_memory   app_map_;
};

void* protector_block_accessor::get_segment_app_ptr() const noexcept
{
    return this->seg_.get_app_ptr();
}

} // namespace medsm
} // namespace menps

