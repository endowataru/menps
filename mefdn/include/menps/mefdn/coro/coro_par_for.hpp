
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/tuple.hpp>
#include <menps/mefdn/assert.hpp>

namespace menps {
namespace mefdn {

template <typename Index, template <typename> class Child, typename T>
struct basic_coro_par_for
{
    template <typename F>
    struct coro : F
    {
        using result_type = void;
        using children = typename F::template define_children<Child>;
        
        explicit coro(
            const Index first
        ,   const Index last
        ,   const Index step
        ,   const T     val
        )
            : first_(first)
            , last_(last)
            , step_(step)
            , middle_(first + (last - first) / 2 / step * step)
            , val_(val)
        {
            MEFDN_ASSERT(this->last_ - this->first_ > 0);
        }
        
        template <typename C>
        struct start : C {
            typename C::return_type operator() () {
                auto& fr = this->frame();
                if (fr.last_ - fr.first_ <= fr.step_) {
                    // Call child coroutine.
                    return this->template call<finish, Child>(fr.first_, fr.val_);
                }
                else {
                    return this->template fork_rec<after_fork, coro>(
                        fr.t_, fr.first_, fr.middle_, fr.step_, fr.val_);
                }
            }
        };
        
    private:
        template <typename C>
        struct after_fork : C {
            typename C::return_type operator() () {
                auto& fr = this->frame();
                return this->template call_rec<after_call, coro>(
                    fr.middle_, fr.last_, fr.step_, fr.val_);
            }
        };
        
        template <typename C>
        struct after_call : C {
            typename C::return_type operator() () {
                auto& fr = this->frame();
                return this->template join<finish>(fr.t_);
            }
        };
        
        template <typename C>
        struct finish : C {
            typename C::return_type operator() () {
                return this->set_return();
            }
        };
        
        typename F::template task<> t_;
        const Index first_;
        const Index last_;
        const Index step_;
        const Index middle_;
        const T     val_;
    };
};

template <typename P>
class basic_par_for_label
{
    MEFDN_DEFINE_DERIVED(P)
    
    using frame_type = typename P::frame_type;
    using worker_type = typename frame_type::worker_type;
    using return_type = typename frame_type::return_type;

public:
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame,
        typename Index, typename... Args>
    return_type parallel_for(
        const Index     first
    ,   const Index     last
    ,   const Index     step
    ,   Args&&...       args
    ) {
        auto& self = this->derived();
        
        return self.template call_rec<
            NextLabel
        ,   basic_coro_par_for<Index, ChildFrame, Args...>::template coro
        >(
            first, last, step
        ,   mefdn::forward<Args>(args)...
        );
    }
};

} // namespace mefdn
} // namespace menps

