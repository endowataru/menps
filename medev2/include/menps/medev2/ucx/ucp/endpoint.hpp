
#pragma once

#include <menps/medev2/ucx/ucp/ucp.hpp>
#include <menps/mefdn/memory/unique_ptr.hpp>
#include <menps/mefdn/external/fmt.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace ucp {

template <typename P>
struct endpoint_deleter
{
    using ucp_facade_type = typename P::ucp_facade_type;
    
    ucp_facade_type* uf;
    
    void operator () (ucp_ep* const ep) const noexcept {
        this->uf->ep_destroy({ ep });
    }
};

template <typename P>
class endpoint
    : public mefdn::unique_ptr<ucp_ep, endpoint_deleter<P>>
{
    using deleter_type = endpoint_deleter<P>;
    using base = mefdn::unique_ptr<ucp_ep, deleter_type>;
    
    using ucp_facade_type = typename P::ucp_facade_type;
    
public:
    endpoint() noexcept = default;
    
    explicit endpoint(ucp_facade_type& uf, ucp_ep* const p)
        : base(p, deleter_type{ &uf })
    { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    endpoint(endpoint&&) noexcept = default;
    endpoint& operator = (endpoint&&) noexcept = default;
    
    static endpoint create(
        ucp_facade_type&                uf
    ,   ucp_worker*                     wk
    ,   const ucp_ep_params_t* const    params
    ) {
        ucp_ep* p = nullptr;
        
        const auto ret = uf.ep_create({ wk, params, &p });
        if (ret != UCS_OK) {
            throw_error("ucp_ep_create", ret);
        }
        
        return endpoint(uf, p);
    }
    
    void put(
        const void* const       buffer
    ,   const mefdn::size_t     length
    ,   const mefdn::uint64_t   remote_addr
    ,   ucp_rkey* const         rkey
    ) {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.put({ this->get(), buffer, length, remote_addr, rkey });
        
        if (MEFDN_UNLIKELY(ret != UCS_OK)) {
            throw_error("ucp_put", ret);
        }
    }
    
    MEFDN_NODISCARD
    ucs_status_ptr_t put_nb(
        const void * const          buffer
    ,   const mefdn::size_t         length
    ,   const mefdn::uint64_t       remote_addr
    ,   const ucp_rkey_h            rkey
    ,   const ucp_send_callback_t   cb
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Call ucp_put_nb()\t"
            "buffer:0x{:x}"
            "length:{}\t"
            "remote_addr:0x{:x}\t"
        ,   reinterpret_cast<mefdn::uintptr_t>(buffer)
        ,   length
        ,   remote_addr
        );
        
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.put_nb({ this->get(), buffer, length, remote_addr, rkey, cb });
        
        if (MEFDN_UNLIKELY(UCS_PTR_IS_ERR(ret))) {
            throw_error("ucp_put_nb", ret);
        }
        
        return ret;
    }
    
    MEFDN_NODISCARD
    ucs_status_ptr_t get_nb(
        void * const                buffer
    ,   const mefdn::size_t         length
    ,   const mefdn::uint64_t       remote_addr
    ,   const ucp_rkey_h            rkey
    ,   const ucp_send_callback_t   cb
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Call ucp_get_nb()\t"
            "buffer:0x{:x}"
            "length:{}\t"
            "remote_addr:0x{:x}\t"
        ,   reinterpret_cast<mefdn::uintptr_t>(buffer)
        ,   length
        ,   remote_addr
        );
        
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.get_nb({ this->get(), buffer, length, remote_addr, rkey, cb });
        
        if (MEFDN_UNLIKELY(UCS_PTR_IS_ERR(ret))) {
            throw_error("ucp_get_nb", ret);
        }
        
        return ret;
    }
    
    
    MEFDN_NODISCARD
    ucs_status_ptr_t atomic_fetch_nb(
        const ucp_atomic_fetch_op_t opcode
    ,   const mefdn::uint64_t       value
    ,   void * const                result
    ,   const mefdn::size_t         op_size
    ,   const mefdn::uint64_t       remote_addr
    ,   const ucp_rkey_h            rkey
    ,   const ucp_send_callback_t   cb
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Call ucp_atomic_fetch_nb()\t"
            "opcode:{}\t"
            "value:{}\t"
            "result:0x{:x}\t"
            "op_size:{}\t"
            "remote_addr:0x{:x}\t"
        ,   opcode
        ,   value
        ,   reinterpret_cast<mefdn::uintptr_t>(result)
        ,   op_size
        ,   remote_addr
        );
        
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.atomic_fetch_nb({ this->get(), opcode, value, result,
                op_size, remote_addr, rkey, cb });
        
        if (MEFDN_UNLIKELY(UCS_PTR_IS_ERR(ret))) {
            throw_error("ucp_atomic_fetch_nb", ret);
        }
        
        return ret;
    }
    
private:
    static void throw_error(
        const char* const   func_name
    ,   const ucs_status_t  ret
    ) {
        throw ucx_error(fmt::format("{}() failed", func_name), ret);
    }
    static void throw_error(
        const char* const       func_name
    ,   const ucs_status_ptr_t  ret
    ) {
        throw ucx_error(fmt::format("{}() failed", func_name), UCS_PTR_STATUS(ret));
    }
};

} // namespace ucp
} // namespace ucx
} // namespace medev2
} // namespace menps

