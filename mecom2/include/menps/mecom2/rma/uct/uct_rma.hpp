
#pragma once

#include <menps/mecom2/rma/rma_itf_id.hpp>
#include <menps/mecom2/rma/uct/basic_uct_completion.hpp>
#include <menps/mecom2/rma/uct/basic_uct_rma.hpp>
#include <menps/mecom2/rma/uct/basic_uct_rma_alloc.hpp>
#include <menps/mecom2/rma/basic_public_rma_ptr.hpp>
#include <menps/mecom2/rma/basic_remote_rma_ptr.hpp>
#include <menps/mecom2/rma/basic_unique_public_ptr.hpp>
#include <menps/mecom2/rma/rma_dlmalloc_allocator.hpp>
#include <menps/mecom2/com/uct/uct_worker_set.hpp>
#include <menps/mefdn/ptr_facade.hpp>

#include <menps/mecom2/rma/alltoall_buffer.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class uct_rma
    : public basic_uct_rma<P>
    , public basic_uct_rma_alloc<P>
    #ifdef MECOM2_RMA_USE_DLMALLOC_ALLOCATOR
    , public rma_dlmalloc_allocator<P>
    #endif
{
    using policy_type = P;
    
    using uct_itf_type = typename policy_type::uct_itf_type;
    using uct_facade_type = typename uct_itf_type::uct_facade_type;
    using memory_domain_type = typename uct_itf_type::memory_domain_type;
    using endpoint_type = typename uct_itf_type::endpoint_type;
    
    using worker_set_type = typename P::worker_set_type;
    
    using uct_rkey_info_type = typename policy_type::uct_rkey_info_type;
    
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
    template <typename T>
    using unique_local_ptr = typename policy_type::template unique_local_ptr<T>;
    
    template <typename U, typename T>
    static U* member(T* const p, U (T::* const q)) noexcept {
        return &p->*q;
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
    explicit uct_rma(Conf&& conf)
        : uf_(conf.uf)
        , component_(conf.component)
        , md_(conf.md)
        , wk_set_(conf.wk_set)
        , md_attr_(conf.md.query())
    {
        #ifdef MECOM2_RMA_USE_DLMALLOC_ALLOCATOR
        this->init_public_allocator(2ull << 30); // TODO: magic number
        #endif
    }
    
    ~uct_rma()
    {
        #ifdef MECOM2_RMA_USE_DLMALLOC_ALLOCATOR
        this->deinit_public_allocator();
        #endif
    }
    
    void progress() {
        // Do nothing.
        // TODO: Is this OK?
    }
    
    uct_facade_type& get_uct_facade() const noexcept {
        return this->uf_;
    }
    uct_component* get_component() const noexcept {
        return this->component_;
    }
    memory_domain_type& get_md() const noexcept {
        return this->md_;
    }
    const uct_md_attr_t get_md_attr() const noexcept {
        return this->md_attr_;
    }
    
    uct_rkey_info_type lock_rma(
        const proc_id_type              proc
    ,   const remote_ptr<const void>&   rptr
    ) {
        const auto wk_num = this->wk_set_.current_worker_num();
        auto* minfo = rptr.get_minfo();
        
        while (!this->wk_set_.try_start_rma(wk_num, proc)) {
            policy_type::ult_itf_type::this_thread::yield();
        }
        
        return {
            wk_num
        ,   proc
        ,   this->wk_set_.get_worker(wk_num)
        ,   this->wk_set_.get_iface(wk_num)
        ,   this->wk_set_.get_ep(wk_num, proc)
        ,   minfo->rkey
        ,   this->wk_set_.get_worker_lock(wk_num)
        };
    }
    void unlock_rma(uct_rkey_info_type& info)
    {
        this->wk_set_.finish_rma(info.wk_num, info.proc);
    }
    
    void flush(const proc_id_type /*proc*/)
    {
        // FIXME: There's no way to accomplish remote completion in UCT
    }
    
private:
    uct_facade_type&    uf_;
    uct_component*      component_;
    memory_domain_type& md_;
    worker_set_type&    wk_set_;
    uct_md_attr_t       md_attr_;
};

template <typename P>
using uct_rma_ptr = mefdn::unique_ptr<uct_rma<P>>;

template <typename P>
inline uct_rma_ptr<P> make_uct_rma(
    typename P::uct_itf_type::uct_facade_type&      uf
,   uct_component* const                            component
,   typename P::uct_itf_type::memory_domain_type&   md
,   typename P::worker_set_type&                    wk_set
) {
    struct conf {
        typename P::uct_itf_type::uct_facade_type&      uf;
        uct_component*                                  component;
        typename P::uct_itf_type::memory_domain_type&   md;
        typename P::worker_set_type&                    wk_set;
    };
    return mefdn::make_unique<uct_rma<P>>(conf{ uf, component, md, wk_set });
}



using worker_num_t = mefdn::size_t;

template <typename UctItf>
struct uct_rma_remote_minfo {
    typename UctItf::remote_key_type    rkey;
};

template <typename UctItf, typename T>
struct uct_remote_ptr_policy;

template <typename UctItf, typename T>
using uct_remote_ptr = basic_remote_rma_ptr<uct_remote_ptr_policy<UctItf, T>>;

template <typename UctItf, typename T>
struct uct_remote_ptr_policy
{
    using derived_type = uct_remote_ptr<UctItf, T>;
    using element_type = T;
    using minfo_type = uct_rma_remote_minfo<UctItf>;
    
    using size_type = mefdn::size_t;
    using difference_type = mefdn::ptrdiff_t;
    
    template <typename U>
    using rebind_t = uct_remote_ptr<UctItf, U>;
    
    using uct_itf_type = UctItf;
    
    using remote_addr_type = mefdn::uint64_t;
};


template <typename UctItf>
struct uct_rma_public_minfo {
    typename UctItf::memory_type        mem;
    typename UctItf::rkey_buffer_type   rkey_buf;
    #ifdef MECOM2_USE_WORKER_LOCAL_ALLOCATOR
    mefdn::size_t                       alloc_id;
    #endif
};

template <typename UctItf, typename T>
struct uct_public_ptr_policy;

template <typename UctItf, typename T>
using uct_public_ptr = basic_public_rma_ptr<uct_public_ptr_policy<UctItf, T>>;

template <typename UctItf, typename T>
struct uct_public_ptr_policy
{
    using derived_type = uct_public_ptr<UctItf, T>;
    using element_type = T;
    using minfo_type = uct_rma_public_minfo<UctItf>;
    
    using size_type = mefdn::size_t;
    using difference_type = mefdn::ptrdiff_t;
    
    template <typename U>
    using rebind_t = uct_public_ptr<UctItf, U>;
};


template <typename UctItf>
struct uct_rma_policy;


template <typename UctItf, typename T>
struct uct_unique_public_ptr_policy {
    using derived_type = basic_unique_public_ptr<uct_unique_public_ptr_policy>;
    using resource_type =
        uct_public_ptr<UctItf, mefdn::remove_extent_t<T>>;
        // TODO: Necessary for arrays, but this should be generalized.
    
    using deleter_type = unique_public_ptr_deleter<uct_unique_public_ptr_policy>;
    
    using allocator_type = uct_rma<uct_rma_policy<UctItf>>;
};

template <typename UctItf, typename T>
using uct_unique_public_ptr = basic_unique_public_ptr<uct_unique_public_ptr_policy<UctItf, T>>;


template <typename UctItf>
struct uct_rkey_info
{
    worker_num_t                                wk_num;
    mefdn::size_t                               proc; // TODO: Use proc_id_type
    typename UctItf::worker_type&               wk;
    typename UctItf::interface_type&            iface;
    typename UctItf::endpoint_type&             ep;
    typename UctItf::remote_key_type&           rkey;
    typename UctItf::ult_itf_type::spinlock&    lock;
};


template <typename UctItf>
struct uct_completion_policy
{
    enum class comp_state_type
    {
        created = 1
    ,   waiting
    ,   finished
    };
    
    using uct_itf_type = UctItf;
    using ult_itf_type = typename UctItf::ult_itf_type;
    
    using atomic_comp_state_type = typename ult_itf_type::template atomic<comp_state_type>;
    
    using rkey_info_type = uct_rkey_info<UctItf>;
};


template <typename T>
struct uct_rma_element_type_of
    : mefdn::type_identity<typename T::element_type>
{ };

template <typename U>
struct uct_rma_element_type_of<U*>
    : mefdn::type_identity<U>
{ };


template <typename UctItf>
struct uct_rma_policy
{
    using derived_type = uct_rma<uct_rma_policy>;
    
    using proc_id_type = mefdn::size_t; // TODO
    using size_type = mefdn::size_t;
    
    using ult_itf_type = typename UctItf::ult_itf_type;
    
    template <typename T>
    using remote_ptr = uct_remote_ptr<UctItf, T>;
    template <typename T>
    using local_ptr = uct_public_ptr<UctItf, T>;
    template <typename T>
    using unique_local_ptr = uct_unique_public_ptr<UctItf, T>;
    template <typename T>
    using public_ptr = uct_public_ptr<UctItf, T>;
    template <typename T>
    using unique_public_ptr = uct_unique_public_ptr<UctItf, T>;
    
    template <typename Ptr>
    using element_type_of = typename uct_rma_element_type_of<Ptr>::type;
    
    using public_minfo_type = uct_rma_public_minfo<UctItf>;
    using remote_minfo_type = uct_rma_remote_minfo<UctItf>;
    
    using rkey_info_type = uct_rkey_info<UctItf>;
    using worker_num_type = worker_num_t;
    
    using uct_itf_type = UctItf;
    
    using worker_set_type = uct_worker_set<uct_worker_set_policy<UctItf>>;
    
    using uct_rkey_info_type = uct_rkey_info<UctItf>;
    
    using completion_type = basic_uct_completion<uct_completion_policy<UctItf>>;
    
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

template <typename UctItf>
struct uct_rma_resource {
    using uct_itf_type = UctItf;
    
    template <typename Coll>
    explicit uct_rma_resource(
        const char* const   tl_name
    ,   const char* const   dev_name
    ,   Coll&               coll
    ) {
        auto om_ret = uct_itf_type::open_md(this->uf, tl_name, dev_name);
        this->component = om_ret.component;
        this->md = mefdn::move(om_ret.md);
        
        auto iface_conf =
            uct_itf_type::iface_config_type::read(
                this->uf, md.get(), tl_name, nullptr, nullptr);
        
        // TODO
        //iface_conf.modify("IB_TX_QUEUE_LEN", "4096");
        
        uct_iface_params_t iface_params = uct_iface_params_t();
        iface_params.field_mask = UCT_IFACE_PARAM_FIELD_OPEN_MODE |
            UCT_IFACE_PARAM_FIELD_DEVICE | UCT_IFACE_PARAM_FIELD_STATS_ROOT |
            UCT_IFACE_PARAM_FIELD_RX_HEADROOM;
        iface_params.open_mode = UCT_IFACE_OPEN_MODE_DEVICE;
        iface_params.mode.device.tl_name = tl_name;
        iface_params.mode.device.dev_name = dev_name;
        iface_params.stats_root = ucs_stats_get_root();
        iface_params.rx_headroom = 0;
        
        this->wk_set =
            mecom2::make_uct_worker_set<uct_itf_type>(
                this->uf, this->md, iface_conf.get(), &iface_params, coll);
        
        this->rma = make_uct_rma<uct_rma_policy<UctItf>>(
            this->uf, this->component, this->md, *this->wk_set);
    }
    
    typename uct_itf_type::uct_facade_type      uf;
    uct_component*                              component;
    typename uct_itf_type::memory_domain_type   md;
    uct_worker_set_ptr<UctItf>                  wk_set;
    uct_rma_ptr<uct_rma_policy<UctItf>>         rma;
};


template <typename UctItf, typename Coll>
inline mefdn::unique_ptr<uct_rma_resource<UctItf>>
make_uct_rma_resource(
    const char* const   tl_name
,   const char* const   dev_name
,   Coll&               coll
) {
    return mefdn::make_unique<uct_rma_resource<UctItf>>(tl_name, dev_name, coll);
}


template <typename P>
struct get_rma_itf_type<rma_itf_id_t::UCT, P>
    : mefdn::type_identity<
        uct_rma<uct_rma_policy<typename P::uct_itf_type>>
    > { };

} // namespace mecom2
} // namespace menps

