
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/ucx/uct/uct_funcs.hpp>
#include <menps/medev2/ucx/ucx_error.hpp>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_uct_worker;

template <typename P>
class proxy_uct_consumer
{
    using ult_itf_type = typename P::ult_itf_type;
    using suspended_thread_type = typename ult_itf_type::suspended_thread;

    using proxy_worker_type = typename P::proxy_worker_type;
    
public:
    using delegated_func_type = typename P::delegated_func_type;

    // TODO: breaking encapsulation
    explicit proxy_uct_consumer(proxy_worker_type& w) : w_(w) { }

    using del_exec_result = fdn::tuple<bool, suspended_thread_type>;
    del_exec_result execute(const delegated_func_type& func) {
        return typename proxy_worker_type::execute_delegated{this->w_}(func);
    }
    using do_progress_result = suspended_thread_type;
    do_progress_result progress() {
        return typename proxy_worker_type::do_progress{this->w_}();
    }

    bool is_active() const noexcept {
        return this->w_.is_active();
    }

private:
    proxy_worker_type& w_;
};

template <typename P>
class proxy_uct_worker
{
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    using orig_uct_facade_type = typename orig_uct_itf_type::uct_facade_type;
    
    using delegator_type = typename P::delegator_type;
    using consumer_type = typename delegator_type::consumer_type;
    using delegated_func_type = typename consumer_type::delegated_func_type;
    
    using command_code_type = typename P::command_code_type;
    using proxy_params_type = typename P::proxy_params_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using suspended_thread_type = typename ult_itf_type::suspended_thread;
    
    using size_type = typename P::size_type;
    
    using proxy_completion_type = typename P::proxy_completion_type;
    using proxy_completion_pool_type = typename P::proxy_completion_pool_type;
    
    using proxy_endpoint_type = typename P::proxy_endpoint_type;
    using proxy_iface_type = typename P::proxy_iface_type;

    friend consumer_type;
    
public:
    using orig_worker_type = typename orig_uct_itf_type::worker_type;
    
    explicit proxy_uct_worker(
        orig_uct_facade_type&   orig_uf
    ,   orig_worker_type        orig_wk
    )
        : orig_uf_(orig_uf)
        , orig_wk_(mefdn::move(orig_wk))
        , con_(*this)
        , del_()
    {
        this->del_.start_consumer(this->con_);
    }
    ~proxy_uct_worker()
    {
        this->del_.stop_consumer();
    }
    
    orig_worker_type& get_orig_worker() {
        return this->orig_wk_;
    }
    orig_uct_facade_type& get_orig_facade() {
        return this->orig_uf_;
    }
    
    void add_ongoing(const size_type num) {
        this->num_ongoing_ += num;
    }
    void remove_ongoing(const size_type num) {
        MEFDN_ASSERT(this->num_ongoing_ >= num);
        this->num_ongoing_ -= num;
    }
    
    size_type get_num_ongoing() const noexcept {
        return this->num_ongoing_;
    }
    bool is_active() const noexcept {
        return this->get_num_ongoing() > 0;
    }
    
private:
    bool is_available() const noexcept {
        return this->get_num_ongoing() < P::max_num_ongoing;
    }

    static proxy_iface_type& get_proxy_iface(uct_iface* const iface)
    {
        const auto pr_iface = reinterpret_cast<proxy_iface_type*>(iface);
        MEFDN_ASSERT(pr_iface != nullptr);
        return *pr_iface;
    }
    static proxy_iface_type& replace_iface(uct_iface** const iface)
    {
        auto& pr_iface = get_proxy_iface(*iface);
        
        auto& orig_iface = pr_iface.get_orig_iface();
        *iface = orig_iface.get();
        
        return pr_iface;
    }
    static proxy_endpoint_type& get_proxy_ep(uct_ep* const ep)
    {
        const auto pr_ep = reinterpret_cast<proxy_endpoint_type*>(ep);
        MEFDN_ASSERT(pr_ep != nullptr);
        return *pr_ep;
    }
    static proxy_endpoint_type& replace_ep(uct_ep** const ep)
    {
        auto& pr_ep = get_proxy_ep(*ep);
        
        auto& orig_ep = pr_ep.get_orig_endpoint();
        *ep = orig_ep.get();
        
        return pr_ep;
    }
    proxy_completion_type* replace_comp(
        uct_completion_t** const        comp
    ,   const uct_completion_callback_t pr_cb
    ,   void* const                     pr_ptr
    ) {
        const auto orig_comp = *comp;
        
        const auto pc =
            this->create_proxy_completion(pr_cb, pr_ptr, orig_comp);
        
        // Replace the parameter "comp".
        *comp = &pc->pr_comp;
        // TODO: Only "real_p.comp" is replaced here (mixture of real / proxy).
        
        return pc;
    }
    
    template <typename Params>
    ucs_status_t execute_iface(
        ucs_status_t (orig_uct_facade_type::*f)(const Params&)
    ,   const Params& proxy_p
    ) {
        auto real_p = proxy_p;
        // Replace the parameter iface.
        this->replace_iface(&real_p.iface);
        
        this->del_.lock();
        
        const auto ret = (this->orig_uf_.*f)(real_p);
        
        this->del_.unlock();
        
        return ret;
    }
    
    template <typename Params>
    ucs_status_t execute_ep(
        ucs_status_t (orig_uct_facade_type::*f)(const Params&)
    ,   const Params& proxy_p
    ) {
        auto real_p = proxy_p;
        // Replace the parameter ep.
        this->replace_ep(&real_p.ep);
        
        this->del_.lock();
        
        const auto ret = (this->orig_uf_.*f)(real_p);
        
        this->del_.unlock();
        
        return ret;
    }
    
    using execute_imm_result = fdn::tuple<bool, suspended_thread_type*>;
    
    template <typename Params>
    struct execute_imm_ep
    {
        proxy_uct_worker& self;
        ucs_status_t (orig_uct_facade_type::*f)(const Params&);
        const Params& proxy_p;
        ucs_status_t* ret_st;
        
        execute_imm_result operator() ()
        {
            if (!self.is_available()) {
                return execute_imm_result{ false, nullptr };
            }
            
            auto real_p = proxy_p;
            // Replace the parameter "ep".
            auto& pr_ep = self.replace_ep(&real_p.ep);
            
            // Call the original function directly.
            const ucs_status_t st = (self.orig_uf_.*f)(real_p);
            
            // Return the status value.
            *ret_st = st;
            
            if (st == UCS_INPROGRESS) {
                // Increment the number of ongoing requests.
                pr_ep.increment_ongoing();
            }
            
            return execute_imm_result{
                st != UCS_ERR_NO_RESOURCE
            ,   nullptr // TODO ?
            };
        }
    };
    
    template <typename Params>
    struct execute_imm_ep_comp
    {
        proxy_uct_worker& self;
        ucs_status_t (orig_uct_facade_type::*f)(const Params&);
        const Params& proxy_p;
        ucs_status_t* ret_st;
        
        execute_imm_result operator() ()
        {
            auto real_p = proxy_p;
            auto& pr_ep = self.get_proxy_ep(proxy_p.ep);
            
            const auto pc =
                self.replace_comp(&real_p.comp, &on_complete_ep_decrement, &pr_ep);
            
            const auto ret =
                execute_imm_ep<Params>{ self, f, real_p, ret_st }();
            
            using fdn::get;
            if (!get<0>(ret)) {
                self.destroy_proxy_completion(pc);
            }
            
            return ret;
        }
    };
    
    template <typename Params>
    struct execute_iface_flush
    {
        proxy_uct_worker& self;
        ucs_status_t (orig_uct_facade_type::*f)(const Params&);
        const Params& proxy_p;
        ucs_status_t* ret_st;
        
        execute_imm_result operator() ()
        {
            auto real_p = proxy_p;
            auto& pr_iface = self.replace_iface(&real_p.iface);
            
            self.replace_comp(&real_p.comp, &on_complete_iface_flush, &pr_iface);
            
            const auto st = (self.orig_uf_.*f)(real_p);
            
            *ret_st = st;
            
            return execute_imm_result{ true, nullptr };
        }
    };
    
    template <typename Params>
    struct execute_ep_flush
    {
        proxy_uct_worker& self;
        ucs_status_t (orig_uct_facade_type::*f)(const Params&);
        const Params& proxy_p;
        ucs_status_t* ret_st;
        
        execute_imm_result operator() ()
        {
            auto real_p = proxy_p;
            auto& pr_ep = self.replace_ep(&real_p.ep);
            
            self.replace_comp(&real_p.comp, &on_complete_ep_flush, &pr_ep);
            
            const auto st = (self.orig_uf_.*f)(real_p);
            
            *ret_st = st;
            
            return execute_imm_result{ true, nullptr };
        }
    };
    
    using delegate_result = suspended_thread_type*;
    
    template <typename Params>
    struct delegate
    {
        command_code_type code;
        Params proxy_params_type::*mem;
        const Params& proxy_p;
        
        delegate_result operator() (delegated_func_type& func)
        {
            func.code = code;
            func.params.*mem = proxy_p;
            return nullptr;
        }
    };
    
    template <template <typename> class Exec, typename Params>
    ucs_status_t execute_or_delegate(
        const command_code_type code
    ,   ucs_status_t (orig_uct_facade_type::*f)(const Params&)
    ,   Params proxy_params_type::*mem
    ,   const Params& proxy_p
    ) {
        ucs_status_t ret_st = UCS_OK;
        
        const bool is_locked =
            this->del_.execute_or_delegate(
                Exec<Params>{
                    *this, f, proxy_p, &ret_st
                }
            ,   delegate<Params>{
                    code, mem, proxy_p
                }
            );
        
        const auto ret =
            is_locked ? ret_st : UCS_INPROGRESS;
        
        return ret;
    }
    
public:
    #define D(method, name, tr, num, ...) \
        tr post_ ## name(const medev2::ucx::uct::name ## _params& proxy_p) { \
            return this->method( \
                &orig_uct_facade_type::name \
            ,   proxy_p \
            ); \
        }
    
    // sync (iface)
    MEDEV2_UCT_IFACE_FUNCS_SYNC_STATUS(D, execute_iface)
    
    // sync (ep)
    MEDEV2_UCT_EP_FUNCS_SYNC_STATUS(D, execute_ep)
    
    #undef D
    
    #define D(method, name, tr, num, ...) \
        tr post_ ## name(const medev2::ucx::uct::name ## _params& proxy_p) { \
            return this->method( \
                command_code_type::name \
            ,   &orig_uct_facade_type::name \
            ,   &proxy_params_type::name \
            ,   proxy_p \
            ); \
        }
    
    // async (ep, post, implicit)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(D, template execute_or_delegate<execute_imm_ep>)
    
    // async (ep, post completion)
    MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(D, template execute_or_delegate<execute_imm_ep_comp>)
    
    // async (iface_flush)
    MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(D, template execute_or_delegate<execute_iface_flush>)
    
    // async (ep_flush)
    MEDEV2_UCT_EP_FUNCS_EP_FLUSH(D, template execute_or_delegate<execute_ep_flush>)
    
    #undef D
    
private:
    using del_exec_result = typename consumer_type::del_exec_result;
    
    struct execute_delegated
    {
        proxy_uct_worker& self;
        
        del_exec_result operator() (const delegated_func_type& func) const
        {
            execute_imm_result ret = execute_imm_result();
            ucs_status_t st = UCS_OK;
            
            switch (func.code) {
                #define D(Exec, name, Name, tr, num, ...) \
                    case command_code_type::name: { \
                        ret = Exec<medev2::ucx::uct::name ## _params>{ \
                            self \
                        ,   &orig_uct_facade_type::name \
                        ,   func.params.name \
                        ,   &st \
                        }(); \
                        break; \
                    }
                
                // async (ep, post, implicit)
                MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_IMPLICIT(D, execute_imm_ep)
                
                // async (ep, post completion)
                MEDEV2_UCT_EP_FUNCS_ASYNC_STATUS_COMPLETION(D, execute_imm_ep_comp)
                
                // async (iface_flush)
                MEDEV2_UCT_IFACE_FUNCS_IFACE_FLUSH(D, execute_iface_flush)
                
                // async (ep_flush)
                MEDEV2_UCT_EP_FUNCS_EP_FLUSH(D, execute_ep_flush)
                
                #undef D
                
                case command_code_type::inv_op:
                default: {
                    MEFDN_LOG_FATAL(
                        "msg:Undefined proxy UCT operation code.\t"
                        "code:{}"
                    ,   static_cast<int>(func.code)
                    );
                    std::abort();
                }
            }
            
            if (!(st == UCS_OK || st == UCS_INPROGRESS || st == UCS_ERR_NO_RESOURCE)) {
                throw medev2::ucx::ucx_error("error in offloading", st);
            }
            
            using fdn::get;
            return del_exec_result{ get<0>(ret), suspended_thread_type() };
        }
    };
    
    using do_progress_result = typename consumer_type::do_progress_result;
    
    struct do_progress
    {
        proxy_uct_worker& self;
        
        do_progress_result operator() () const
        {
            bool is_polled MEFDN_MAYBE_UNUSED = false;
            while (self.orig_wk_.progress() != 0) {
                // Poll until there are completions
                is_polled = true;
            }
            #ifdef MEQDC_UCT_ENABLE_YIELD
            if (!is_polled) {
                ult_itf_type::this_thread::yield();
            }
            #endif
            
            return suspended_thread_type();
        }
    };
    
    static void on_complete_ep_decrement(
        uct_completion_t* const comp
    ,   const ucs_status_t      status
    ) {
        const auto pc = proxy_uct_worker::to_proxy_completion(comp);
        
        const auto ep = static_cast<proxy_endpoint_type*>(pc->pr_ptr);
        MEFDN_ASSERT(ep != nullptr);
        ep->decrement_ongoing();
        
        proxy_uct_worker::on_complete_common(pc, status);
    }
    static void on_complete_ep_flush(
        uct_completion_t* const comp
    ,   const ucs_status_t      status
    ) {
        const auto pc = proxy_uct_worker::to_proxy_completion(comp);
        
        const auto ep = static_cast<proxy_endpoint_type*>(pc->pr_ptr);
        MEFDN_ASSERT(ep != nullptr);
        ep->flush_ongoing();
        
        proxy_uct_worker::on_complete_common(pc, status);
    }
    static void on_complete_iface_flush(
        uct_completion_t* const comp
    ,   const ucs_status_t      status
    ) {
        const auto pc = proxy_uct_worker::to_proxy_completion(comp);
        
        const auto iface = static_cast<proxy_iface_type*>(pc->pr_ptr);
        MEFDN_ASSERT(iface != nullptr);
        iface->flush_ongoing();
        
        proxy_uct_worker::on_complete_common(pc, status);
    }
    
    static void on_complete_common(
        proxy_completion_type* const    pc
    ,   const ucs_status_t              status
    ) {
        auto& self = *pc->self;
        MEFDN_ASSERT(pc->self != nullptr);
        
        const auto orig_comp = pc->orig_comp;
        MEFDN_ASSERT(orig_comp != nullptr);
        
        MEFDN_LOG_VERBOSE(
            "msg:Complete communication in offloading thread.\t"
            "num_ongoing:{}"
        ,   self.get_num_ongoing()
        );
        
        // Instead of the original runtime,
        // this layer calls the user-defined function here.
        if (--orig_comp->count == 0) {
            orig_comp->func(orig_comp, status);
        }
        
        self.destroy_proxy_completion(pc);
    }
    
    
    proxy_completion_type* create_proxy_completion(
        const uct_completion_callback_t pr_cb
    ,   void* const                     pr_ptr
    ,   uct_completion_t* const         orig_comp
    ) {
        const auto pc = this->pc_pool_.allocate();
        pc->self = this;
        pc->pr_comp.count = 1;
        pc->pr_comp.func = pr_cb;
        pc->pr_ptr = pr_ptr;
        pc->orig_comp = orig_comp;
        return pc;
    }
    void destroy_proxy_completion(proxy_completion_type* const pc) {
        this->pc_pool_.deallocate(pc);
    }
    static proxy_completion_type* to_proxy_completion(uct_completion_t* const comp) {
        MEFDN_ASSERT(comp != nullptr);
        
        // Calculate the pointer to the whole struct.
        return mefdn::get_container_of(comp, &proxy_completion_type::pr_comp);
    }
    
    orig_uct_facade_type&   orig_uf_;
    orig_worker_type        orig_wk_;
    consumer_type           con_;
    delegator_type          del_;
    size_type               num_ongoing_ = 0;
    
    proxy_completion_pool_type  pc_pool_;
};

} // namespace meqdc
} // namespace menps

