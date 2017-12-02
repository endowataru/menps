
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/container/circular_buffer.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class blocking_tag_allocator
{
    using tag_type = typename P::tag_type;
    
public:
    template <typename Conf>
    explicit blocking_tag_allocator(Conf&& conf)
    {
        const auto tag_start = conf.reply_tag_start;
        const auto tag_end = conf.reply_tag_end;
        tags_.set_capacity(tag_end - tag_start);
        for (tag_type t = tag_start; t < tag_end; ++t) {
            tags_.push_back(t);
        }
    }
    
    tag_type allocate_tag()
    {
        // FIXME: thread safety
        const auto t = this->tags_.front();
        tags_.pop_front();
        return t;
    }
    
    void deallocate_tag(const tag_type t)
    {
        // FIXME: thread safety
        tags_.push_front(t);
    }
    
private:
    mefdn::circular_buffer<tag_type> tags_;
};

} // namespace mecom2
} // namespace menps

