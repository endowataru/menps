
#pragma once

#include <menps/mecom2/rpc/basic_message.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
struct default_message_deleter
{
    void operator() (void* const ptr)
    {
        using base = basic_message<P>;
        
        const auto data = static_cast<mefdn::byte*>(ptr);
        const auto head = data - base::header_size();
        
        delete[] head;
    }
};

template <typename P>
class default_message_allocator
{
    using size_type = typename P::size_type;
    
    using server_request_message_type = typename P::template server_request_message<void>;
    using client_request_message_type = typename P::template client_request_message<void>;
    using server_reply_message_type = typename P::template server_reply_message<void>;
    using client_reply_message_type = typename P::template client_reply_message<void>;
    
public:
    server_request_message_type allocate_server_request(const size_type data_size)
    {
        return default_message_allocator::allocate<server_request_message_type>(data_size);
    }
    client_request_message_type allocate_client_request(const size_type data_size)
    {
        return default_message_allocator::allocate<client_request_message_type>(data_size);
    }
    server_reply_message_type allocate_server_reply(const size_type data_size)
    {
        return default_message_allocator::allocate<server_reply_message_type>(data_size);
    }
    client_reply_message_type allocate_client_reply(const size_type data_size)
    {
        return default_message_allocator::allocate<client_reply_message_type>(data_size);
    }
    
private:
    template <typename Msg>
    Msg allocate(const size_type data_size)
    {
        const auto header_size = Msg::header_size();
        const auto total_size = header_size + data_size;
        const auto header_ptr = new mefdn::byte[total_size];
        const auto data_ptr = header_ptr + header_size;
        return Msg(data_ptr, data_size);
    }
};

} // namespace mecom2
} // namespace menps

