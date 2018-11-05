
#pragma once

#include <menps/meqdc/common.hpp>
#include <menps/medev2/mpi/mpi_funcs.hpp>
#include <menps/mefdn/force_integer_cast.hpp>
#include <menps/mefdn/atomic.hpp>
#include <menps/mefdn/logger.hpp>

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
    using qdlock_delegator_type = typename P::qdlock_delegator_type;
    using qdlock_pool_type = typename qdlock_delegator_type::qdlock_pool_type;
    using qdlock_node_type = typename qdlock_delegator_type::qdlock_node_type;
    using proxy_params_type = typename P::proxy_params_type;
    using size_type = typename P::size_type;
    
public:
    explicit proxy_mpi_facade(
        int* const      argc
    ,   char*** const   argv
    )
        : orig_mf_(argc, argv)
        , req_pool_(P::max_num_requests)
        , req_hld_()
        , qd_pool_()
        , qd_(this->qd_pool_)
    {
        this->qd_.start_consumer(
            [this] (qdlock_node_type& n) {
                return this->execute_delegated(n);
            }
        ,   [this] {
                return this->do_progress();
            }
        );
    }
    
    ~proxy_mpi_facade()
    {
        this->qd_.stop_consumer();
    }
    
private:
    template <typename Params>
    struct execute_imm_nb
    {
        proxy_mpi_facade& self;
        void (orig_mpi_facade_type::*f)(const Params&);
        const Params& real_p;
        proxy_request_type* proxy_req_ptr;
        
        bool operator() ()
        {
            (self.orig_mf_.*f)(real_p);
            
            if (proxy_req_ptr != nullptr)
            {
                self.req_hld_.add(proxy_req_ptr, proxy_req_ptr->orig_req);
            }
            
            return self.is_active();
        }
    };
    
    template <typename Params>
    struct delegate_nb
    {
        proxy_mpi_facade& self;
        command_code_type code;
        Params proxy_params_type::*mem;
        const Params& real_p;
        proxy_request_type* proxy_req_ptr;
        
        void operator() (qdlock_node_type& cur)
        {
            cur.func.code = code;
            cur.func.params.*mem = real_p;
            cur.func.proxy_req = proxy_req_ptr;
        }
    };
    
    template <typename Params>
    void execute_or_delegate_req_nb(
        const command_code_type code
    ,   void (orig_mpi_facade_type::*f)(const Params&)
    ,   Params proxy_params_type::*mem
    ,   const Params& proxy_p
    ) {
        const auto proxy_req_ptr = this->req_pool_.allocate();
        
        auto real_p = proxy_p;
        real_p.request = &proxy_req_ptr->orig_req;
        
        *proxy_p.request = this->to_mpi_request(proxy_req_ptr);
        
        this->qd_.execute_or_delegate(
            execute_imm_nb<Params>{ *this, f, real_p, proxy_req_ptr }
        ,   delegate_nb<Params>{ *this, code, mem, real_p, proxy_req_ptr }
        );
    }
    
    template <typename Params>
    void execute_or_delegate_noreq_nb(
        const command_code_type code
    ,   void (orig_mpi_facade_type::*f)(const Params&)
    ,   Params proxy_params_type::*mem
    ,   const Params& real_p
    ) {
        this->qd_.execute_or_delegate(
            execute_imm_nb<Params>{ *this, f, real_p, nullptr }
        ,   delegate_nb<Params>{ *this, code, mem, real_p, nullptr }
        );
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
            ); \
        }
    
    MEDEV2_MPI_P2P_NONBLOCK_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_REQ_BASED_RMA_FUNCS(D, /*dummy*/)
    
    #undef D
    
    // non-request-based non-blocking calls
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& proxy_p) { \
            this->execute_or_delegate_noreq_nb( \
                command_code_type::name \
            ,   &orig_mpi_facade_type::name \
            ,   &proxy_params_type::name \
            ,   proxy_p \
            ); \
        }
    
    MEDEV2_MPI_RMA_FUNCS(D, /*dummy*/)
    
    #undef D
    
    #undef EXECUTE_OR_DELEGATE
    
    // blocking calls with non-blocking alternative
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& p) { \
            MPI_Request proxy_req = MPI_Request(); \
            medev2::mpi::i##name##_params nb_proxy_p{ \
                MEDEV2_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__) \
            ,   &proxy_req \
            }; \
            this->i##name(nb_proxy_p); \
            this->wait(&proxy_req); \
        }
    
    MEDEV2_MPI_P2P_BLOCK_FUNCS(D, /*dummy*/) 
    
    #undef D
    
    // blocking calls without non-blocking alternative
    
    #define D(dummy, name, Name, tr, num, ...) \
        void name(const medev2::mpi::name##_params& real_p) { \
            this->qd_.lock(); \
            this->orig_mf_.name(real_p); \
            this->qd_.unlock(); \
        }
    
    MEDEV2_MPI_COLLECTIVE_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_RMA_WIN_FUNCS(D, /*dummy*/)
    MEDEV2_MPI_OTHER_FUNCS(D, /*dummy*/)
    
    #undef D
    
    void test(const medev2::mpi::test_params& proxy_p)
    {
        const auto proxy_req =
            this->to_proxy_request_pointer(*proxy_p.request);
        
        const auto state = proxy_req->state.load(mefdn::memory_order_acquire);
        
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
    
    void wait(const medev2::mpi::wait_params& proxy_p)
    {
        const auto proxy_req =
            this->to_proxy_request_pointer(*proxy_p.request);
        
        bool needs_wait = false;
        
        auto state = proxy_req->state.load(mefdn::memory_order_acquire);
        
        if (state == proxy_request_state_type::finished) {
            // Don't need to wait for the request.
        }
        else {
            MEFDN_ASSERT(state == proxy_request_state_type::created);
            
            needs_wait =
                proxy_req->state.compare_exchange_strong(
                    state
                ,   proxy_request_state_type::waiting
                ,   mefdn::memory_order_acq_rel
                ,   mefdn::memory_order_relaxed
                );
            
            if (needs_wait) { MEFDN_ASSERT(state == proxy_request_state_type::created); }
            else            { MEFDN_ASSERT(state == proxy_request_state_type::finished); }
        }
        
        if (needs_wait) {
            proxy_req->uv.wait();
        }
        
        if (proxy_p.status != MPI_STATUS_IGNORE) {
            *proxy_p.status = proxy_req->status;
        }
        
        this->req_pool_.deallocate(proxy_req);
    }
    
    void progress()
    {
        // Do nothing.
        
        MEFDN_LOG_VERBOSE("msg:Called proxy MPI progress (do nothing).");
    }
    
private:
    bool execute_delegated(const qdlock_node_type& n)
    {
        auto& func = n.func;
        
        switch (func.code) {
            #define D(dummy, name, Name, tr, num, ...) \
                case command_code_type::name: { \
                    proxy_mpi_facade::execute_imm_nb<medev2::mpi::name##_params>{ \
                        *this \
                    ,   &orig_mpi_facade_type::name \
                    ,   func.params.name \
                    ,   func.proxy_req \
                    }(); \
                    break; \
                }
            
            // request-based non-blocking calls
            MEDEV2_MPI_P2P_NONBLOCK_FUNCS(D, /*dummy*/)
            MEDEV2_MPI_REQ_BASED_RMA_FUNCS(D, /*dummy*/)
            
            // non-request-based non-blocking calls
            MEDEV2_MPI_RMA_FUNCS(D, /*dummy*/)
            
            #undef D
        }
        
        return this->is_active();
    }
    
    bool do_progress()
    {
        MEFDN_LOG_VERBOSE("msg:Entering progress of proxy MPI.");
        
        this->req_hld_.progress(this->orig_mf_, complete_request());
        
        MEFDN_LOG_VERBOSE("msg:Exiting progress of proxy MPI.");
        
        return this->is_active();
    }
    
    struct complete_request
    {
        void operator() (
            proxy_request_type* const   proxy_req
        ,   const MPI_Status&           status
        ) {
            proxy_req->status = status;
            
            bool waiting = false;
            
            auto state = proxy_req->state.load(mefdn::memory_order_acquire);
            
            if (state == proxy_request_state_type::waiting) {
                waiting = true;
            }
            else {
                MEFDN_ASSERT(state == proxy_request_state_type::created);
                
                waiting =
                    ! proxy_req->state.compare_exchange_strong(
                        state
                    ,   proxy_request_state_type::finished
                    ,   mefdn::memory_order_acq_rel
                    ,   mefdn::memory_order_relaxed
                    );
                
                if (waiting) { MEFDN_ASSERT(state == proxy_request_state_type::waiting); }
                else         { MEFDN_ASSERT(state == proxy_request_state_type::created); }
            }
            
            if (waiting) {
                // Wake the waiting thread.
                // Prefer returning to the progress thread immediately.
                proxy_req->uv.notify_signal();
            }
        }
    };
    
    bool is_active() const noexcept
    {
        return this->req_hld_.get_num_ongoing() > 0;
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
    
    orig_mpi_facade_type    orig_mf_;
    request_pool_type       req_pool_;
    request_holder_type     req_hld_;
    qdlock_pool_type        qd_pool_;
    qdlock_delegator_type   qd_;
};

} // namespace meqdc
} // namespace menps

