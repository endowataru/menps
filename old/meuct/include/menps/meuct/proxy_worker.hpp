
#pragma once

#include <menps/meuct/proxy_funcs.hpp>

namespace menps {
namespace meuct {

template <typename P>
class proxy_worker
{
    using worker_thread_type = typename P::worker_thread_type;
    using command_type = typename P::command_type;
    using command_code_type = typename P::command_code_type;
    using command_queue_type = typename P::command_queue_type;
    
    using orig_uct_itf_type = typename P::orig_uct_itf_type;
    using orig_uct_facade_type = typename orig_uct_itf_type::uct_facade_type;
    using ult_itf_type = typename P::ult_itf_type;
    
    using size_type = typename P::size_type;
    
public:
    using orig_worker_type = typename orig_uct_itf_type::worker_type;
    
    explicit proxy_worker(
        orig_uct_facade_type&   orig_uf
    ,   orig_worker_type        orig_wk
    )
        : orig_uf_(orig_uf)
        , orig_wk_(mefdn::move(orig_wk))
        , que_()
        , wk_th_(*this, que_)
    { }
    
    orig_worker_type& get_orig_worker() {
        return this->orig_wk_;
    }
    orig_uct_facade_type& get_orig_facade() {
        return this->orig_uf_;
    }
    
    #define D(dummy, name, tr, num, ...) \
        tr execute_ ## name(const medev2::ucx::uct::name ## _params& p) { \
            return this->template execute<tr>( \
                command_code_type::name, p); \
        }
    
    MEUCT_UCT_PROXY_FUNCS_SYNC(D, /*dummy*/)
    
    #undef D
    
    #define D(dummy, name, tr, num, ...) \
        tr post_ ## name(const medev2::ucx::uct::name ## _params& p) { \
            this->template post( \
                command_code_type::name, nullptr, p); \
            \
            return UCS_INPROGRESS; \
        }
    
    MEUCT_UCT_PROXY_FUNCS_ASYNC(D, /*dummy*/)
    
    #undef D
    
    void add_ongoing(const size_type num) {
        this->num_ongoing_ += num;
    }
    void remove_ongoing(const size_type num) {
        MEFDN_ASSERT(this->num_ongoing_ >= num);
        this->num_ongoing_ -= num;
    }
    
    size_type get_num_ongoing() {
        return this->num_ongoing_;
    }
    
private:
    template <typename T, typename Params>
    T execute(const command_code_type code, const Params& params)
    {
        typename ult_itf_type::template async_channel<T> ch;
        this->post(code, &ch, params);
        return ch.get(&ult_itf_type::this_thread::yield);
    }
    
    template <typename Params>
    void post(const command_code_type code, void* notif, const Params& params)
    {
        while (true) {
            auto t = this->que_.try_enqueue(1, true);
            
            if (MEFDN_LIKELY(t.valid()))
            {
                auto& dest = *t.begin();
                
                dest.code = code;
                dest.notif = notif;
                
                MEFDN_STATIC_ASSERT(sizeof(Params) <= command_type::params_size);
                reinterpret_cast<Params&>(dest.params) = params;
                
                t.commit(1);
                
                this->que_.notify_if_sleeping(t);
                
                return;
            }
            
            ult_itf_type::this_thread::yield();
        }
    }
    
    orig_uct_facade_type    orig_uf_;
    orig_worker_type        orig_wk_;
    command_queue_type      que_;
    worker_thread_type      wk_th_;
    size_type               num_ongoing_ = 0;
};

} // namespace meuct
} // namespace menps

#define MEUCT_DEFINE_PROXY_WORKER_API(prefix, proxy_worker_type, uf) \
    extern "C" \
    ucs_status_t prefix ## worker_create( \
        ucs_async_context_t* const  async \
    ,   const ucs_thread_mode_t     thread_mode \
    ,   uct_worker_h* const         worker_p \
    ) { \
        auto orig_wk = \
            proxy_worker_type::orig_worker_type::create(uf, async, thread_mode); \
        \
        const auto pr_wk = \
            new proxy_worker_type(uf, menps::mefdn::move(orig_wk)); \
        \
        *worker_p = reinterpret_cast<uct_worker_h>(pr_wk); \
        \
        return UCS_OK; \
    } \
    \
    extern "C" \
    void prefix ## worker_destroy( \
        const uct_worker_h worker \
    ) { \
        auto pr_wk = reinterpret_cast<proxy_worker_type*>(worker); \
        \
        delete pr_wk; \
    }

