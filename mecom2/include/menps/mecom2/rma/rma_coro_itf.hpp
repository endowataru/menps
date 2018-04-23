
#pragma once

#include <menps/mecom2/common.hpp>

namespace menps {
namespace mecom2 {

template <typename P>
class rma_coro_itf
{
    MEFDN_DEFINE_DERIVED(P)
    
    using handle_type   = typename P::handle_type;
    using proc_id_type  = typename P::proc_id_type;
    using size_type     = typename P::size_type;
    
public:
    template <typename F>
    struct coro_flush : F
    {
        using result_type = void;
        
        explicit coro_flush(handle_type h)
            : h_(mefdn::move(h))
        { }
        
        template <typename C>
        struct start : C {
            typename C::return_type operator() () {
                auto& fr = this->frame();
                auto& wk = this->worker();
                auto& st = wk.get_rma_state();
                
                fr.sn_ = st.generate_sn();
                
                return this->template yield<finish>();
            }
        };
        
    private:
        template <typename C>
        struct finish : C {
            typename C::return_type operator() () {
                auto& fr = this->frame();
                auto& wk = this->worker();
                auto& st = wk.get_rma_state();
                
                st.flush(fr.sn_, [&fr] { fr.h_.flush(); });
                
                return this->template set_return();
            }
        };
        
        using worker_type = typename F::worker;
        using rma_state_type = typename worker_type::rma_state_type;
        using rma_sn_type = typename rma_state_type::rma_sn_type;
        
        handle_type h_;
        rma_sn_type sn_;
    };
    
    template <typename F>
    struct coro_read : F
    {
        using result_type = void;
        using children = typename F::template define_children<coro_flush>;
        
        template <typename RemotePtr, typename LocalPtr>
        explicit coro_read(
            derived_type&       self
        ,   const proc_id_type  src_proc
        ,   RemotePtr&&         src_rptr
        ,   LocalPtr&&          dest_lptr
        ,   const size_type     num_elems
        )
            : h_(self.make_handle())
        {
            this->h_.read_nb(
                src_proc
            ,   mefdn::forward<RemotePtr>(src_rptr)
            ,   mefdn::forward<LocalPtr>(dest_lptr)
            ,   num_elems
            );
        }
        
        template <typename C>
        struct start : C {
            typename C::return_type operator() () {
                auto& fr = this->frame();
                return this->template call<finish, coro_flush>(mefdn::move(fr.h_));
            }
        };
        
    private:
        template <typename C>
        struct finish : C {
            typename C::return_type operator() () {
                return this->set_return();
            }
        };
        
        handle_type h_;
    };
};

} // namespace mecom2
} // namespace menps

