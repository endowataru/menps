
#pragma once

#include <menps/mefdn/utility.hpp>

namespace menps {
namespace mefdn {

template <typename... Rs>
struct klt_task
{
    std::thread th;
    mefdn::tuple<Rs...> ret;
};

class klt_worker
{
public:
    template <typename... Ts>
    using task = klt_task<Ts...>;
    
    template <typename T>
    using return_t = T;
    
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame,
        typename Label, typename Task, typename... Args>
    typename Label::return_type fork(Label& lb, Task& tk, Args&&... args)
    {
        using child_frame_type =
            typename Label::template child_frame_t<ChildFrame, fork_ret_cont>;
        
        tk.th = std::thread(
            fork_start<child_frame_type, Task>{ *this, tk }
        ,   mefdn::forward<Args>(args)...
        );
        return lb.template jump<NextLabel>();
    }
    
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame,
        typename Label, typename Task, typename... Args>
    typename Label::return_type fork_rec(Label& lb, Task& tk, Args&&... args)
    {
        using child_frame_type =
            typename Label::template child_frame_t<ChildFrame, fork_ret_cont>;
        
        tk.th = std::thread(
            fork_start<child_frame_type, Task>{ *this, tk }
        ,   mefdn::forward<Args>(args)...
        );
        return lb.template jump<NextLabel>();
    }
    
    template <template <typename> class NextLabel,
        typename Label, typename Task, typename... Args>
    typename Label::return_type join(Label& lb, Task& tk)
    {
        tk.th.join();
        return lb.template jump_with_tuple<NextLabel>(tk.ret);
    }
    
private:
    struct fork_ret_cont {
        template <typename F, typename U>
        U operator() (F&& /*ignored*/, U&& v) const /*may throw*/ {
            return mefdn::forward<U>(v);
        }
    };
    
    template <typename ChildFrame, typename Task>
    struct fork_start {
        klt_worker& wk;
        Task&       tk;
        
        template <typename... Args>
        void operator() (Args&&... args)
        {
            ChildFrame fr(
                mefdn::forward_as_tuple(mefdn::forward<Args>(args)...)
            ,   mefdn::forward_as_tuple()
            );
            tk.ret = mefdn::forward_as_tuple( fr(wk) );
        }
    };
};

} // namespace mefdn
} // namespace menps

