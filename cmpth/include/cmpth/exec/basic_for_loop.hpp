
#pragma once

#include <cmpth/common.hpp>

namespace cmpth {

template <typename P>
class basic_for_loop
{
    using thread_type = typename P::thread_type;
    
public:
    struct execution {
        using sequenced_policy = fdn::execution::sequenced_policy;
        using parallel_policy = fdn::execution::parallel_policy;
        using parallel_unsequenced_policy = fdn::execution::parallel_unsequenced_policy;
        
        static constexpr const sequenced_policy seq{};
        static constexpr const parallel_policy par{};
        static constexpr const parallel_unsequenced_policy par_seq{};
    };
    
    template <typename ExecutionPolicy, typename I, typename... Rest>
    static void for_loop(ExecutionPolicy&& exec,
        fdn::type_identity_t<I> first, I last, Rest&& ... rest)
    {
        const I stride = 1;
        basic_for_loop::for_loop_strided(
            fdn::forward<ExecutionPolicy>(exec), first, last, stride
        ,   fdn::forward<Rest>(rest)...
        );
    }
    
    template <typename I, typename S, typename F>
    static void for_loop_strided(typename execution::sequenced_policy /*seq*/,
        fdn::type_identity_t<I> start, I finish, S stride, F func)
    {
        for ( ; start < finish ; start += stride) {
            func(start);
        }
    }
    
    template <typename I, typename S, typename F>
    static void for_loop_strided(typename execution::parallel_policy /*par*/,
        fdn::type_identity_t<I> first, I last, S stride, const F& func)
    {
        if (last - first == 0) {
            return;
        }
        
        for_loop_par_params<I, S, F> params{
            first,
            0, (last - first + stride - 1) / stride,
            stride, func
        };
        basic_for_loop::for_loop_par_aux<I, S, F>(&params);
    }
    
private:
    template <typename I, typename S, typename F>
    struct for_loop_par_params {
        I first; I a; I b; S stride; const F& func;
    };
    
    template <typename I, typename S, typename F>
    static void for_loop_par_aux(void* const arg)
    {
        try {
            using params_type = for_loop_par_params<I, S, F>;
            const auto& params = static_cast<const params_type*>(arg);
            
            const auto first = params->first;
            const auto a = params->a;
            const auto b = params->b;
            const auto stride = params->stride;
            const auto& func = params->func;
            
            const auto d = b - a;
            MEFDN_ASSERT(d > 0);
            
            if (d == 1) {
                func(first + a * stride);
            }
            else {
                const auto c = a + d / 2;
                params_type p0{ first, a, c, stride, func };
                auto t = thread_type::ptr_fork(
                    &basic_for_loop::for_loop_par_aux<I, S, F>, &p0);
                
                params_type p1{ first, c, b, stride, func };
                basic_for_loop::for_loop_par_aux<I, S, F>(&p1);
                
                t.join();
            }
        }
        catch (std::exception& e) {
            CMPTH_P_LOG_FATAL(P,
                "msg:An exception thrown in mth::for_loop(par)!\t"
            ,   1
            ,   "what", e.what()
            );
            throw;
        }
        catch (...) {
            CMPTH_P_LOG_FATAL(P, "msg:An unknown exception thrown in mth::for_loop(par)!", 0);
            throw;
        }
        
        //return nullptr;
    }
};

template <typename P>
constexpr const fdn::execution::sequenced_policy basic_for_loop<P>::execution::seq;
template <typename P>
constexpr const fdn::execution::parallel_policy basic_for_loop<P>::execution::par;
template <typename P>
constexpr const fdn::execution::parallel_unsequenced_policy basic_for_loop<P>::execution::par_seq;

} // namespace cmpth

