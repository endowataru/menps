
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/mpi/mpi_funcs.hpp>
#include <menps/mefdn/logger.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace meqdc {

template <typename P>
class proxy_mpi_facade
{
    using orig_mpi_facade_type = typename P::orig_mpi_facade_type;
    using request_pool_type = typename P::request_pool_type;
    using request_holder_type = typename P::request_holder_type;
    using command_code_type = typename P::command_code_type;
    using proxy_request_type = typename P::proxy_request_type;
    using proxy_request_state_type = typename P::proxy_request_state_type;
    using delegator_type = typename P::delegator_type;
    using sync_node_type = typename delegator_type::sync_node_type;
    using proxy_params_type = typename P::proxy_params_type;
    using size_type = typename P::size_type;
    
    using ult_itf_type = typename P::ult_itf_type;
    using suspended_thread_type = typename ult_itf_type::suspended_thread;
    using worker_type = typename ult_itf_type::worker;
    
public:
    explicit proxy_mpi_facade(
        int* const      argc
    ,   char*** const   argv
    ,   const int       /*required*/ // TODO: ignored
    ,   int* const      provided
    )
        : orig_mf_(argc, argv, MPI_THREAD_SERIALIZED, &orig_level_)
        , req_pool_(P::max_num_requests)
        , req_hld_()
        , del_()
    {
        *provided = MPI_THREAD_MULTIPLE;
        
        this->del_.start_consumer(
            [this] (sync_node_type& n) {
                return this->execute_delegated(n);
            }
        ,   [this] {
                return this->do_progress();
            }
        );
    }
    
    ~proxy_mpi_facade()
    {
        this->del_.stop_consumer();
    }
    
private:
    struct execute_imm_result
    {
        bool    is_executed;
        bool    is_active;
        suspended_thread_type*  wait_sth;
    };
    
    template <typename Params>
    struct execute_imm_nb
    {
        proxy_mpi_facade& self;
        void (orig_mpi_facade_type::*f)(const Params&);
        const Params& real_p;
        proxy_request_type* proxy_req_ptr;
        bool needs_wait;
        
        execute_imm_result operator() ()
        {
            // Check if it is allowed to establish a new request.
            const bool is_executed =
                self.req_hld_.is_available();
            
            bool do_wait = needs_wait;
            
            if (is_executed) {
                // Call the original function directly.
                (self.orig_mf_.*f)(real_p);
                
                if (proxy_req_ptr != nullptr)
                {
                    if (proxy_req_ptr->orig_req == MPI_REQUEST_NULL) {
                        // When the MPI function returns MPI_REQUEST_NULL,
                        // this code treats it as the request completion.
                        // This behavior is observed in calling MPI_Ibarrier()
                        // with a single process communicator on Intel MPI.
                        proxy_req_ptr->state.store(
                            proxy_request_state_type::finished
                        ,   ult_itf_type::memory_order_relaxed
                        );
                        do_wait = false;
                    }
                    else {
                        self.req_hld_.add(proxy_req_ptr, proxy_req_ptr->orig_req);
                    }
                }
            }
            
            if (do_wait) {
                proxy_req_ptr->state.store(
                    proxy_request_state_type::waiting
                ,   ult_itf_type::memory_order_relaxed
                );
                return { is_executed, self.is_active(), &proxy_req_ptr->sth };
            }
            else {
                return { is_executed, self.is_active(), nullptr };
            }
        }
    };
    
    struct delegate_result {
        suspended_thread_type*  wait_sth;
    };
    
    template <typename Params>
    struct delegate_nb
    {
        proxy_mpi_facade& self; // TODO: unused
        command_code_type code;
        Params proxy_params_type::*mem;
        const Params& real_p;
        proxy_request_type* proxy_req_ptr;
        bool needs_wait;
        
        delegate_result operator() (sync_node_type& cur)
        {
            cur.func.code = code;
            cur.func.params.*mem = real_p;
            cur.func.proxy_req = proxy_req_ptr;
            if (needs_wait) {
                proxy_req_ptr->state.store(
                    proxy_request_state_type::waiting
                ,   ult_itf_type::memory_order_relaxed
                );
                return { &proxy_req_ptr->sth };
            }
            else {
                return { nullptr };
            }
        }
    };
    
    template <typename Params>
    void execute_or_delegate_req_nb(
        const command_code_type code
    ,   void (orig_mpi_facade_type::*f)(const Params&)
    ,   Params proxy_params_type::*mem
    ,   const Params& proxy_p
    ,   const bool needs_wait
    ) {
        const auto proxy_req_ptr = this->req_pool_.allocate();
        
        auto real_p = proxy_p;
        real_p.request = &proxy_req_ptr->orig_req;
        
        *proxy_p.request = this->to_mpi_request(proxy_req_ptr);
        
        this->del_.execute_or_delegate(
            execute_imm_nb<Params>{ *this, f, real_p, proxy_req_ptr, needs_wait }
        ,   delegate_nb<Params>{ *this, code, mem, real_p, proxy_req_ptr, needs_wait }
        );
    }
    
    template <typename Params>
    void execute_or_delegate_noreq_nb(
        const command_code_type code
    ,   void (orig_mpi_facade_type::*f)(const Params&)
    ,   Params proxy_params_type::*mem
    ,   const Params& real_p
    ,   const bool needs_wait
    ) {
        this->del_.execute_or_delegate(
            execute_imm_nb<Params>{ *this, f, real_p, nullptr, needs_wait }
        ,   delegate_nb<Params>{ *this, code, mem, real_p, nullptr, needs_wait }
        );
    }
    
    template <typename Params>
    MEFDN_NODISCARD
    proxy_request_type* execute_or_delegate_req_b(
        const command_code_type code
    ,   void (orig_mpi_facade_type::*f)(const Params&)
    ,   Params proxy_params_type::*mem
    ,   const Params& proxy_p
    ) {
        const auto proxy_req_ptr = this->req_pool_.allocate();
        
        auto real_p = proxy_p;
        real_p.request = &proxy_req_ptr->orig_req;
        
        *proxy_p.request = this->to_mpi_request(proxy_req_ptr);
        
        this->del_.execute_or_delegate(
            execute_imm_nb<Params>{ *this, f, real_p, proxy_req_ptr, true }
        ,   delegate_nb<Params>{ *this, code, mem, real_p, proxy_req_ptr, true }
        );
        
        return proxy_req_ptr;
    }
    
public:
    // request-based non-blocking calls
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& proxy_p) { \
            this->execute_or_delegate_req_nb( \
                command_code_type::name \
            ,   &orig_mpi_facade_type::name \
            ,   &proxy_params_type::name \
            ,   proxy_p \
            ,   false \
            ); \
        }
    
    MEDEV2_MPI_P2P_NONBLOCK_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_REQ_BASED_RMA_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_COLLECTIVE_NONBLOCK_FUNCS(D, /*dummy*/)
    
    #undef D
    
    // non-request-based non-blocking calls
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& proxy_p) { \
            this->execute_or_delegate_noreq_nb( \
                command_code_type::name \
            ,   &orig_mpi_facade_type::name \
            ,   &proxy_params_type::name \
            ,   proxy_p \
            ,   false \
            ); \
        }
    
    MEDEV2_MPI_RMA_FUNCS(D, /*dummy*/)
    
    #undef D
    
    #undef EXECUTE_OR_DELEGATE
    
    // blocking calls with non-blocking alternative
    
    #if 1
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& p) { \
            MPI_Request proxy_req = MPI_Request(); \
            medev2::mpi::i##name##_params nb_proxy_p{ \
                MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__) \
            ,   &proxy_req \
            }; \
            const auto proxy_req_ptr = \
                this->execute_or_delegate_req_b( \
                    command_code_type::i##name \
                ,   &orig_mpi_facade_type::i##name \
                ,   &proxy_params_type::i##name \
                ,   nb_proxy_p \
                ); \
            this->req_pool_.deallocate(proxy_req_ptr); \
        }
    
    MEDEV2_MPI_P2P_BLOCK_SEND_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_COLLECTIVE_BLOCK_FUNCS(D, /*dummy*/)
    
    #undef D
    
    void recv(const medev2::mpi::recv_params& p) {
        MPI_Request proxy_req = MPI_Request();
        medev2::mpi::irecv_params nb_proxy_p{
            p.buf, p.count, p.datatype, p.source, p.tag, p.comm
            // Note: p.status is not listed here.
        ,   &proxy_req
        };
        const auto proxy_req_ptr =
            this->execute_or_delegate_req_b(
                command_code_type::irecv
            ,   &orig_mpi_facade_type::irecv
            ,   &proxy_params_type::irecv
            ,   nb_proxy_p
            );
        
        if (p.status != MPI_STATUS_IGNORE) {
            *p.status = proxy_req_ptr->status;
        }
        
        this->req_pool_.deallocate(proxy_req_ptr);
    }
    
    #else
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& p) { \
            MPI_Request proxy_req = MPI_Request(); \
            medev2::mpi::i##name##_params nb_proxy_p{ \
                MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__) \
            ,   &proxy_req \
            }; \
            this->i##name(nb_proxy_p); \
            this->wait({ &proxy_req, MPI_STATUS_IGNORE }); \
        }
    
    MEDEV2_MPI_P2P_BLOCK_SEND_FUNCS(D, /*dummy*/) 
    
    #undef D
    
    void recv(const medev2::mpi::recv_params& p) {
        MPI_Request proxy_req = MPI_Request();
        medev2::mpi::irecv_params nb_proxy_p{
            p.buf, p.count, p.datatype, p.source, p.tag, p.comm
            // Note: p.status is not listed here.
        ,   &proxy_req
        };
        this->irecv(nb_proxy_p);
        this->wait({ &proxy_req, p.status });
    }
    #endif
    
    // blocking calls without non-blocking alternative
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& real_p) { \
            this->del_.lock(); \
            this->orig_mf_.name(real_p); \
            this->del_.unlock(); \
        }
    
    MEDEV2_MPI_PROBE_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_RMA_WIN_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_OTHER_FUNCS(D, /*dummy*/)
    
    #undef D
    
    void test(const medev2::mpi::test_params& proxy_p)
    {
        const auto proxy_req =
            this->to_proxy_request_pointer(*proxy_p.request);
        
        const auto state = proxy_req->state.load(ult_itf_type::memory_order_acquire);
        
        if (state == proxy_request_state_type::finished) {
            *proxy_p.request = MPI_REQUEST_NULL;
            *proxy_p.flag = true;
            if (proxy_p.status != MPI_STATUS_IGNORE) {
                *proxy_p.status = proxy_req->status;
            }
            
            // This thread removes this request.
            // Be careful that all other threads including the consumer thread
            // don't access the request any more.
            this->req_pool_.deallocate(proxy_req);
        }
        else {
            MEFDN_ASSERT(state == proxy_request_state_type::created);
            *proxy_p.flag = false;
        }
    }
    
    void testsome(const medev2::mpi::testsome_params& proxy_p)
    {
        const auto incount = proxy_p.incount;
        int outcount = 0;
        for (int i = 0; i < incount; ++i) {
            const auto req_ptr = &proxy_p.array_of_requests[i];
            int flag = 0;
            MPI_Status status = MPI_Status();
            this->test({ req_ptr, &flag, &status });
            if (flag) {
                proxy_p.array_of_indices[outcount] = i;
                proxy_p.array_of_statuses[outcount] = status;
                ++outcount;
            }
        }
        *proxy_p.outcount = outcount;
    }
    
private:
    struct wait_func {
        bool operator () (
            worker_type&                /*wk*/
        ,   proxy_request_type* const   proxy_req
        ,   bool* const                 is_blocked
        ) const noexcept
        {
            auto expected = proxy_request_state_type::created;
            
            if (proxy_req->state.compare_exchange_strong(
                expected
            ,   proxy_request_state_type::waiting
            ,   ult_itf_type::memory_order_acq_rel
            ,   ult_itf_type::memory_order_relaxed
            )) {
                return true;
            }
            else {
                *is_blocked = false;
                return false;
            }
        }
    };
    
public:
    void wait(const medev2::mpi::wait_params& proxy_p)
    {
        const auto proxy_req =
            this->to_proxy_request_pointer(*proxy_p.request);
        
        auto state = proxy_req->state.load(ult_itf_type::memory_order_acquire);
        
        if (state == proxy_request_state_type::finished) {
            // Don't need to wait for the request.
        }
        else {
            MEFDN_ASSERT(state == proxy_request_state_type::created);
            
            // Block until the completion.
            bool is_blocked = true;
            proxy_req->sth.template wait_with<wait_func>(proxy_req, &is_blocked);
        }
        
        if (proxy_p.status != MPI_STATUS_IGNORE) {
            *proxy_p.status = proxy_req->status;
        }
        
        this->req_pool_.deallocate(proxy_req);
    }
    
    void progress()
    {
        #if 1
        this->del_.lock();
        this->orig_mf_.progress();
        this->del_.unlock();
        
        #else
        // Do nothing.
        
        MEFDN_LOG_VERBOSE("msg:Called proxy MPI progress (do nothing).");
        #endif
    }
    
private:
    struct del_exec_result {
        bool    is_executed;
        bool    is_active;
        suspended_thread_type   awake_sth;
    };
    
    del_exec_result execute_delegated(const sync_node_type& n)
    {
        execute_imm_result ret = execute_imm_result();
        
        auto& func = n.func;
        
        switch (func.code) {
            #define D(dummy, name, Name, tr, num, ...) \
                case command_code_type::name: { \
                    ret = proxy_mpi_facade::execute_imm_nb<medev2::mpi::name##_params>{ \
                        *this \
                    ,   &orig_mpi_facade_type::name \
                    ,   func.params.name \
                    ,   func.proxy_req \
                    ,   false \
                    }(); \
                    break; \
                }
            
            // request-based non-blocking calls
            MEDEV2_MPI_P2P_NONBLOCK_FUNCS(D, /*dummy*/)
            MEDEV2_MPI_REQ_BASED_RMA_FUNCS(D, /*dummy*/)
            MEDEV2_MPI_COLLECTIVE_NONBLOCK_FUNCS(D, /*dummy*/)
            
            // non-request-based non-blocking calls
            MEDEV2_MPI_RMA_FUNCS(D, /*dummy*/)
            
            #undef D
            
            case command_code_type::inv_op:
            default: {
                MEFDN_LOG_FATAL(
                    "msg:Undefined proxy MPI operation code.\t"
                    "code:{}"
                ,   static_cast<int>(func.code)
                );
                std::abort();
            }
        }
        
        return { ret.is_executed, ret.is_active, suspended_thread_type() };
    }
    
    struct do_progress_result {
        bool is_active;
        suspended_thread_type   awake_sth;
    };
    
    do_progress_result do_progress()
    {
        MEFDN_LOG_VERBOSE("msg:Entering progress of proxy MPI.");
        
        auto ret = this->req_hld_.progress(this->orig_mf_, complete_request());
        
        #ifdef MEQDC_MPI_ENABLE_ALWAYS_PROGRESS
        this->orig_mf_.progress();
        #else
        #ifdef MEQDC_MPI_ENABLE_STATIC_OFFLOADING
        if (ret.num_ongoing == 0) {
            this->orig_mf_.progress();
        }
        #endif
        #endif
        
        MEFDN_LOG_VERBOSE("msg:Exiting progress of proxy MPI.");
        
        return { this->is_active(), mefdn::move(ret.awake_sth) };
    }
    
    struct complete_request
    {
        MEFDN_NODISCARD
        bool operator() (
            proxy_request_type* const   proxy_req
        ,   const MPI_Status&           status
        ) {
            proxy_req->status = status;
            
            bool waiting = false;
            
            auto state = proxy_req->state.load(ult_itf_type::memory_order_acquire);
            
            if (state == proxy_request_state_type::waiting) {
                waiting = true;
            }
            else {
                MEFDN_ASSERT(state == proxy_request_state_type::created);
                
                waiting =
                    ! proxy_req->state.compare_exchange_strong(
                        state
                    ,   proxy_request_state_type::finished
                    ,   ult_itf_type::memory_order_acq_rel
                    ,   ult_itf_type::memory_order_relaxed
                    );
                
                if (waiting) { MEFDN_ASSERT(state == proxy_request_state_type::waiting); }
                else         { MEFDN_ASSERT(state == proxy_request_state_type::created); }
            }
            
            // If there's a thread waiting for this request,
            // the suspended thread of "proxy_req" is awaken by the callee.
            return waiting;
        }
    };
    
    bool is_active() const noexcept
    {
        #ifdef MEQDC_MPI_ENABLE_STATIC_OFFLOADING
        return true;
        #else
        return this->req_hld_.get_num_ongoing() > 0;
        #endif
    }
    
    proxy_request_type* to_proxy_request_pointer(const MPI_Request req) {
        #if 1
        const auto num = mefdn::force_integer_cast<size_type>(req) - P::mpi_request_offset;
        return this->req_pool_.to_pointer(num);
        #else
        return reinterpret_cast<proxy_request_type*>(req);
        #endif
    }
    
    MPI_Request to_mpi_request(proxy_request_type* const proxy_req) {
        #if 1
        const auto num = this->req_pool_.to_number(proxy_req);
        return mefdn::force_integer_cast<MPI_Request>(num + P::mpi_request_offset);
        #else
        return reinterpret_cast<proxy_request_type*>(req);
        #endif
    }
    
    int                     orig_level_;
    orig_mpi_facade_type    orig_mf_;
    request_pool_type       req_pool_;
    request_holder_type     req_hld_;
    delegator_type          del_;
};

} // namespace meqdc
} // namespace menps

