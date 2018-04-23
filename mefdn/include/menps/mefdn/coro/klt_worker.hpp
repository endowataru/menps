
#pragma once

#include <menps/mefdn/coro/coro.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/thread.hpp>

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
            typename Label::template child_frame_t<ChildFrame, identity_retcont>;
        
        using worker_type = typename Label::worker_type;
        auto& wk = lb.worker();
        
        tk.th = std::thread(
            fork_start<child_frame_type, Task, worker_type>{ wk, tk }
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
            typename Label::template child_frame_t<ChildFrame, identity_retcont>;
        
        using worker_type = typename Label::worker_type;
        auto& wk = lb.worker();
        
        tk.th = std::thread(
            fork_start<child_frame_type, Task, worker_type>{ wk, tk }
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
    
    template <template <typename> class NextLabel,
        typename Label>
    typename Label::return_type yield(Label& lb)
    {
        std::this_thread::yield();
        return lb.template jump<NextLabel>();
    }
    
private:
    // Worker is a template parameter
    // because it may be a derived class of this class.
    template <typename ChildFrame, typename Task, typename Worker>
    struct fork_start {
        Worker& wk;
        Task&   tk;
        
        template <typename... Args>
        void operator() (Args&&... args)
        {
            ChildFrame fr(
                mefdn::forward_as_tuple(mefdn::forward<Args>(args)...)
            ,   mefdn::forward_as_tuple()
            );
            // Call fr(wk) and assign a tuple as a returned value.
            tk.ret = mefdn::call_and_make_tuple(fr, wk);
        }
    };
};

} // namespace mefdn
} // namespace menps

