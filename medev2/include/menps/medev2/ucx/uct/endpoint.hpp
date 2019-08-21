
#pragma once

#include <menps/medev2/ucx/uct/uct.hpp>
#include <menps/mefdn/logger.hpp>

namespace menps {
namespace medev2 {
namespace ucx {
namespace uct {

template <typename P>
struct endpoint_deleter
{
    using uct_facade_type = typename P::uct_facade_type;
    
    uct_facade_type* uf;
    
    void operator () (uct_ep* const p) const noexcept {
        uf->ep_destroy({ p });
    }
};

template <typename P>
class endpoint
    : public mefdn::unique_ptr<uct_ep, endpoint_deleter<P>>
{
    using deleter_type = endpoint_deleter<P>;
    using base = mefdn::unique_ptr<uct_ep, deleter_type>;
    
    using uct_facade_type = typename P::uct_facade_type;
    
public:
    endpoint() noexcept = default;
    
    explicit endpoint(
        uct_facade_type&    uf
    ,   uct_ep* const       p
    )
        : base(p, deleter_type{ &uf })
    { }
    
    endpoint(const endpoint&) = delete;
    endpoint& operator = (const endpoint&) = delete;
    
    endpoint(endpoint&&) noexcept = default;
    endpoint& operator = (endpoint&&) noexcept = default;
    
    // UCT_IFACE_FLAG_CONNECT_TO_EP
    static endpoint create(
        uct_facade_type&                uf
    ,   const uct_ep_params_t* const    params
    ) {
        uct_ep* p = nullptr;
        
        const auto ret = uf.ep_create({ params, &p });
        if (ret != UCS_OK) {
            throw ucx_error("uct_ep_create() failed", ret);
        }
        
        return endpoint(uf, p);
    }
    // for convenience
    static endpoint create(
        uct_facade_type&    uf
    ,   uct_iface* const    iface
    ) {
        uct_ep_params_t params = uct_ep_params_t();
        params.field_mask = UCT_EP_PARAM_FIELD_IFACE;
        params.iface = iface;
        
        return endpoint::create(uf, &params);
    }

    void get_ep_address(uct_ep_addr_t* const addr)
    {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret = uf.ep_get_address({ this->get(), addr });
        if (ret != UCS_OK) {
            throw ucx_error("uct_ep_get_address() failed", ret);
        }
    }
    void connect_to_ep(
        const uct_device_addr_t* const  dev_addr
    ,   const uct_ep_addr_t* const      ep_addr
    ) {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_connect_to_ep({ this->get(), dev_addr, ep_addr });
        
        if (ret != UCS_OK) {
            throw ucx_error("uct_ep_connect_to_ep() failed", ret);
        }
    }
    
    ucs_status_t put_short(
        const void * const      buffer
    ,   const unsigned int      length
    ,   const mefdn::uint64_t   remote_addr
    ,   const uct_rkey_t        rkey
    ) {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_put_short({ this->get(), buffer, length, remote_addr, rkey });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_put_short() failed", ret);
        }
        
        return ret;
    }
    
    ssize_t put_bcopy(
        const uct_pack_callback_t   pack_cb
    ,   void* const                 arg
    ,   const mefdn::uint64_t       remote_addr
    ,   const uct_rkey_t            rkey
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Call uct_ep_put_bcopy()\t"
            "arg:0x{:x}\t"
            "remote_addr:{}\t"
        ,   reinterpret_cast<mefdn::uintptr_t>(arg)
        ,   remote_addr
        );
        
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_put_bcopy({ this->get(), pack_cb, arg, remote_addr, rkey });
        
        if (ret < 0) {
            throw ucx_error("uct_ep_put_bcopy() failed",
                static_cast<ucs_status_t>(ret));
        }
        
        return ret;
    }
    
    
    ucs_status_t put_zcopy(
        const uct_iov_t* const      iov
    ,   const mefdn::size_t         iovcnt
    ,   const mefdn::uint64_t       remote_addr
    ,   const uct_rkey_t            rkey
    ,   uct_completion_t* const     comp
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Call uct_ep_put_zcopy()\t"
            "remote_addr:0x{:x}"
        ,   remote_addr
        );
        
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_put_zcopy({ this->get(), iov, iovcnt, remote_addr, rkey, comp });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_put_zcopy() failed", ret);
        }
        
        return ret;
    }

    ucs_status_t get_bcopy(
        const uct_unpack_callback_t unpack_cb
    ,   void * const                arg
    ,   const mefdn::size_t         length
    ,   const mefdn::uint64_t       remote_addr
    ,   const uct_rkey_t            rkey
    ,   uct_completion_t* const     comp
    ) {
        auto& uf = * this->get_deleter().uf;
        
        MEFDN_LOG_VERBOSE(
            "msg:Call uct_ep_get_bcopy()\t"
            "arg:0x{:x}\t"
            "length:{}\t"
            "remote_addr:0x{:x}\t"
        ,   reinterpret_cast<mefdn::uintptr_t>(arg)
        ,   length
        ,   remote_addr
        );
        
        const auto ret =
            uf.ep_get_bcopy({ this->get(), unpack_cb, arg, length,
                remote_addr, rkey, comp });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_get_bcopy() failed", ret);
        }
        
        return ret;
    }
    
    ucs_status_t get_zcopy(
        const uct_iov_t* const  iov
    ,   const mefdn::size_t     iovcnt
    ,   const mefdn::uint64_t   remote_addr
    ,   const uct_rkey_t        rkey
    ,   uct_completion_t* const comp
    ) {
        MEFDN_LOG_VERBOSE(
            "msg:Call uct_ep_get_zcopy()\t"
            "remote_addr:0x{:x}"
        ,   remote_addr
        );
        
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_get_zcopy({ this->get(), iov, iovcnt, remote_addr, rkey, comp });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_get_zcopy() failed", ret);
        }
        
        return ret;
    }
    
    #if 0
    void get_short(
        void * const            buffer
    ,   const unsigned int      length
    ,   const mefdn::uint64_t   remote_addr
    ,   const uct_rkey_t        rkey
    ) {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_get_short({ this->get(), buffer, length, remote_addr, rkey });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_get_short() failed", ret);
        }
    }
    #endif
    
    
    
    ucs_status_t atomic_cswap64(
        const mefdn::uint64_t   compare
    ,   const mefdn::uint64_t   swap
    ,   const mefdn::uint64_t   remote_addr
    ,   const uct_rkey_t        rkey
    ,   mefdn::uint64_t* const  result
    ,   uct_completion_t* const comp
    ) {
        auto& uf = * this->get_deleter().uf;
        
        MEFDN_LOG_VERBOSE(
            "msg:Call uct_ep_atomic_cswap64()\t"
            "compare:{}\t"
            "swap:{}\t"
            "remote_addr:0x{:x}\t"
            "result:0x{:x}"
        ,   compare
        ,   swap
        ,   remote_addr
        ,   reinterpret_cast<mefdn::uintptr_t>(result)
        );
        
        const auto ret =
            uf.ep_atomic_cswap64({
                this->get()
            ,   compare
            ,   swap
            ,   remote_addr
            ,   rkey
            ,   result
            ,   comp
            });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_atomic_cswap64() failed", ret);
        }
        
        return ret;
    }
    
    ucs_status_t flush(
        const unsigned int          flags
    ,   uct_completion_t * const    comp
    ) {
        auto& uf = * this->get_deleter().uf;
        
        const auto ret =
            uf.ep_flush({ this->get(), flags, comp });
        
        if (UCS_STATUS_IS_ERR(ret)) {
            throw ucx_error("uct_ep_flush() failed", ret);
        }
        
        return ret;
    }
};

} // namespace uct
} // namespace ucx
} // namespace medev2
} // namespace menps

