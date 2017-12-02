
#pragma once

#include <menps/mecom2/common.hpp>
#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mecom2 {

template <typename P, bool IsVoid>
class server_context_reply
{
    MEFDN_DEFINE_DERIVED(P)
    
    using server_reply_message_type = typename P::server_reply_message_type;
    
public:
    template <typename... Args>
    server_reply_message_type make_reply(Args&&... args) const {
        auto& self = this->derived();
        auto& s = self.server();
        
        using data_type = typename server_reply_message_type::data_type;
        
        auto msg = s.allocate_server_reply( sizeof(data_type) );
        
        // Construct with the specified parameters.
        new (msg.get()) data_type(mefdn::forward<Args>(args)...);
        
        return mefdn::move(msg)
            .template reinterpret_cast_to<data_type>();
    }
};

template <typename P>
class server_context_reply<P, true> { };

template <typename P>
class server_context
    : public server_context_reply<P,
        mefdn::is_void<typename P::server_reply_message_type::data_type>::value
    >
{
    MEFDN_DEFINE_DERIVED(P)
    
    using server_type = typename P::server_type;
    using server_request_message_type = typename P::server_request_message_type;
    using server_reply_message_type = typename P::server_reply_message_type;
    
    using request_type = typename server_request_message_type::data_type;
    
    using process_id_type = typename P::process_id_type;
    
public:
    using return_type = server_reply_message_type;
    
    /*implicit*/ server_context(
        server_type&                server
    ,   const process_id_type       src_proc
    ,   server_request_message_type rqst_msg
    )
        : server_(server)
        , src_proc_(src_proc)
        , rqst_msg_(mefdn::move(rqst_msg))
    { }
    
    // getter functions
    
    const request_type& request() const noexcept {
        return *this->request_ptr();
    }
    const request_type* request_ptr() const noexcept {
        return this->rqst_msg_.get();
    }
    
    mefdn::size_t size_in_bytes() const noexcept {
        return this->rqst_msg_.size_in_bytes();
    }
    
    process_id_type src_proc() const noexcept {
        return this->src_proc_;
    }
    
    server_type& server() const noexcept {
        return this->server_;
    }
    
private:
    server_type&                server_;
    process_id_type             src_proc_;
    server_request_message_type rqst_msg_;
};

} // namespace mecom2
} // namespace menps

