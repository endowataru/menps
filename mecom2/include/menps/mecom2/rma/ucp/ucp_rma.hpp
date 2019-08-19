
#pragma once

#include <menps/mecom2/rma/ucp/basic_ucp_rma.hpp>
#include <menps/mecom2/rma/ucp/basic_ucp_rma_alloc.hpp>
#include <menps/mecom2/rma/basic_public_rma_ptr.hpp>
#include <menps/mecom2/rma/basic_remote_rma_ptr.hpp>
#include <menps/mecom2/rma/basic_unique_public_ptr.hpp>
#include <menps/mecom2/com/ucp/ucp_worker_set.hpp>
#include <menps/mecom2/rma/rma_itf_id.hpp>
#include <menps/mefdn/ptr_facade.hpp>

namespace menps {
namespace mecom2 {

using rma_ucp_policy = default_ucp_policy;

struct ucp_rma_remote_minfo {
    mefdn::unique_ptr<rma_ucp_policy::remote_key_type []>   rkeys;
};

template <typename T>
struct ucp_remote_ptr_policy;

template <typename T>
using ucp_remote_ptr = basic_remote_rma_ptr<ucp_remote_ptr_policy<T>>;

template <typename T>
struct ucp_remote_ptr_policy
{
    using derived_type = ucp_remote_ptr<T>;
    using element_type = T;
    using minfo_type = ucp_rma_remote_minfo;
    
    using size_type = mefdn::size_t;
    using difference_type = mefdn::ptrdiff_t;
    
    template <typename U>
    using rebind_t = ucp_remote_ptr<U>;
    
    using ucp_itf_type = rma_ucp_policy;
    
    using remote_addr_type = mefdn::uint64_t;
};


struct ucp_rma_public_minfo {
    rma_ucp_policy::memory_type         mem;
    rma_ucp_policy::packed_rkey_type    rkey_buf;
};

template <typename T>
struct ucp_public_ptr_policy;

template <typename T>
using ucp_public_ptr = basic_public_rma_ptr<ucp_public_ptr_policy<T>>;

template <typename T>
struct ucp_public_ptr_policy
{
    using derived_type = ucp_public_ptr<T>;
    using element_type = T;
    using minfo_type = ucp_rma_public_minfo;
    
    using size_type = mefdn::size_t;
    using difference_type = mefdn::ptrdiff_t;
    
    template <typename U>
    using rebind_t = ucp_public_ptr<U>;
};

class ucp_rma;

template <typename T>
struct ucp_unique_public_ptr_policy {
    using derived_type = basic_unique_public_ptr<ucp_unique_public_ptr_policy>;
    using resource_type =
        ucp_public_ptr< mefdn::remove_extent_t<T> >;
        // TODO: Necessary for arrays, but this should be generalized.
    
    using deleter_type = unique_public_ptr_deleter<ucp_unique_public_ptr_policy>;
    
    using allocator_type = ucp_rma;
};

template <typename T>
using ucp_unique_public_ptr = basic_unique_public_ptr<ucp_unique_public_ptr_policy<T>>;



struct ucp_rkey_info
{
    rma_ucp_policy::worker_type&        wk;
    rma_ucp_policy::endpoint_type&      ep;
    rma_ucp_policy::remote_key_type&    rkey;
};

class ucp_rma;


template <typename T>
struct ucp_rma_element_type_of
    : mefdn::type_identity<typename T::element_type>
{ };

template <typename U>
struct ucp_rma_element_type_of<U*>
    : mefdn::type_identity<U>
{ };

struct ucp_rma_policy
{
    using derived_type = ucp_rma;
    
    using proc_id_type = mefdn::size_t;
    using size_type = mefdn::size_t;
    
    using ult_itf_type = default_ult_itf;
    
    template <typename T>
    using remote_ptr = ucp_remote_ptr<T>;
    template <typename T>
    using local_ptr = T*;
    template <typename T>
    using unique_local_ptr = mefdn::unique_ptr<T>;
    template <typename T>
    using public_ptr = ucp_public_ptr<T>; 
    template <typename T>
    using unique_public_ptr = ucp_unique_public_ptr<T>;
    
    template <typename Ptr>
    using element_type_of = typename ucp_rma_element_type_of<Ptr>::type;
    
    using public_minfo_type = ucp_rma_public_minfo;
    using remote_minfo_type = ucp_rma_remote_minfo;
    
    using rkey_info_type = ucp_rkey_info;
    using worker_num_type = mefdn::size_t;
    
    using ucp_itf_type = rma_ucp_policy;
    
    template <typename U, typename Ptr>
    static auto static_cast_to(const Ptr& ptr)
        -> decltype(ptr.template static_cast_to<U>())
    {
        return ptr.template static_cast_to<U>();
    }
    
    static void throw_error(const char* const msg, const ucs_status_t status) {
        throw medev2::ucx::ucx_error(msg, status);
    }
};

class ucp_rma
    : public basic_ucp_rma<ucp_rma_policy>
    , public basic_ucp_rma_alloc<ucp_rma_policy>
{
    using policy_type = ucp_rma_policy;
    
    using ucp_itf_type = typename policy_type::ucp_itf_type;
    using ucp_facade_type = typename ucp_itf_type::ucp_facade_type;
    using context_type = typename ucp_itf_type::context_type;
    using endpoint_type = typename ucp_itf_type::endpoint_type;
    
    using worker_set_type = ucp_worker_set;
    using worker_num_type = typename policy_type::worker_num_type;
    
public:
    using proc_id_type = typename policy_type::proc_id_type;
    using size_type = typename policy_type::size_type;
    
    template <typename T>
    using remote_ptr = typename policy_type::template remote_ptr<T>;
    template <typename T>
    using local_ptr = typename policy_type::template local_ptr<T>;
    template <typename T>
    using public_ptr = typename policy_type::template public_ptr<T>;
    template <typename T>
    using unique_public_ptr = typename policy_type::template unique_public_ptr<T>;
    
    template <typename U, typename T>
    static U* member(T* const p, U (T::* const q)) noexcept {
        return &(p->*q);
    }
    template <typename U, typename T, typename Ptr,
        typename = typename mefdn::enable_if_t<
            ! mefdn::is_pointer<Ptr>::value
        >>
    static auto member(const Ptr& ptr, U (T::* const q))
        -> decltype(ptr.member(q))
    {
        return ptr.member(q);
    }
    
    template <typename Conf>
    explicit ucp_rma(Conf&& conf)
        : uf_(conf.uf)
        , ctx_(conf.ctx)
        , wk_set_(conf.wk_set)
    { }
    
    void progress() {
        // Do nothing.
        // TODO: Is this OK?
    }
    
    ucp_facade_type& get_ucp_facade() const noexcept {
        return this->uf_;
    }
    context_type& get_context() const noexcept {
        return this->ctx_;
    }
    
    ucp_rkey_info get_rkey_info(
        const proc_id_type              proc
    ,   const remote_ptr<const void>&   rptr
    ) {
        const auto wk_num = this->wk_set_.current_worker_num();
        auto* minfo = rptr.get_minfo();
        
        return {
            this->wk_set_.get_worker(wk_num)
        ,   this->wk_set_.get_ep(wk_num, proc)
        ,   minfo->rkeys[wk_num]
        };
    }
    mefdn::size_t get_num_workers()
    {
        return this->wk_set_.get_num_workers();
    }
    endpoint_type& get_ep(
        const worker_num_type   wk_num
    ,   const proc_id_type      proc 
    ) {
        return this->wk_set_.get_ep(wk_num, proc);
    }
    
private:
    ucp_facade_type& uf_;
    context_type& ctx_;
    worker_set_type& wk_set_;
};


using ucp_rma_ptr = mefdn::unique_ptr<ucp_rma>;

inline ucp_rma_ptr make_ucp_rma(
    ucp_rma_policy::ucp_itf_type::ucp_facade_type&  uf
,   ucp_rma_policy::ucp_itf_type::context_type&     ctx
,   ucp_worker_set&                                 wk_set
) {
    struct conf {
        ucp_rma_policy::ucp_itf_type::ucp_facade_type&  uf;
        ucp_rma_policy::ucp_itf_type::context_type&     ctx;
        ucp_worker_set&                                 wk_set;
    };
    return mefdn::make_unique<ucp_rma>(conf{ uf, ctx, wk_set });
}

} // namespace mecom2
} // namespace menps

