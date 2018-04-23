
#pragma once

#include <menps/mefdn/coro/coro.hpp>
#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/tuple.hpp>
#include <menps/mefdn/type_traits/integer_sequence.hpp>

namespace menps {
namespace mefdn {

template <template <typename> class Frame, typename RetCont, typename Worker>
struct sfc_frame_policy;

template <template <typename> class Frame, typename RetCont, typename Worker>
using sfc_frame = basic_coro_frame<sfc_frame_policy<Frame, RetCont, Worker>>;

template <template <typename> class Label,
    template <typename> class Frame, typename RetCont, typename Worker>
struct sfc_label_policy;

template <template <typename> class Label,
    template <typename> class Frame, typename RetCont, typename Worker>
using sfc_label = coro_label<sfc_label_policy<Label, Frame, RetCont, Worker>>;

template <typename Worker>
class sfc_frame_base
{
public:
    // Stackful coroutines need not to track children frames.
    template <template <typename> class... ChildFrames>
    using define_children = void;
    
    using children = define_children<>;
    
    template <typename... Results>
    using task = typename Worker::template task<Results...>;
    
    using worker = Worker;
};

template <template <typename> class Frame, typename RetCont, typename Worker>
struct sfc_frame_policy
{
    using derived_type = sfc_frame<Frame, RetCont, Worker>;
    using user_frame_type = Frame<sfc_frame_base<Worker>>;
    using ret_cont_type = RetCont;
    using worker_type = Worker;
    using start_label_type =
        sfc_label<user_frame_type::template start, Frame, RetCont, Worker>;
};


template <template <typename> class ContLabel,
    template <typename> class Frame, typename RetCont, typename Worker>
class sfc_cont
{
    using frame_type = sfc_frame<Frame, RetCont, Worker>;
    using return_type = typename frame_type::return_type;
    
public:
    explicit sfc_cont(frame_type& fr)
        : fr_(fr)
    { }
    
    template <typename ChildFrame, typename... Args>
    return_type operator() (ChildFrame&& ch_lb, Args&&... args) {
        auto& wk = ch_lb.get_worker();
        
        sfc_label<ContLabel, Frame, RetCont, Worker> lb(this->fr_, wk);
        return lb(mefdn::forward<Args>(args)...);
    }
    
private:
    frame_type& fr_;
};

struct sfc_cont_rec
{
    // This is just an identity function.
    
    template <typename ChildFrame, typename T>
    T operator() (ChildFrame&& /*ignored*/, T&& v) {
        return mefdn::forward<T>(v);
    }
};

template <typename P>
class sfc_label_base
    : public basic_coro_label<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = basic_coro_label<P>;
    
public:
    using typename base::worker_type;
    using typename base::return_type;
    
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame, typename... Args>
    return_type call(Args&&... args)
    {
        auto& self = this->derived();
        auto& fr = self.get_frame();
        auto& wk = self.get_worker();
        
        using ret_cont_type = typename P::template next_cont_t<NextLabel>;
        
        // Construct the child frame directly on this thread's stack.
        sfc_frame<ChildFrame, ret_cont_type, worker_type> cf(
            mefdn::forward_as_tuple(mefdn::forward<Args>(args)...)
        ,   mefdn::forward_as_tuple(fr)
        );
        
        return cf(wk);
    }
    
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame, typename... Args>
    return_type call_rec(Args&&... args)
    {
        auto& self = this->derived();
        auto& wk = self.get_worker();
        
        // Construct the child frame directly on this thread's stack.
        sfc_frame<ChildFrame, sfc_cont_rec, worker_type> cf(
            mefdn::forward_as_tuple(mefdn::forward<Args>(args)...)
        ,   mefdn::forward_as_tuple()
        );
        
        // ... and then jump to the next label with the child's result.
        return self.template jump<NextLabel>(
            // Execute the child first.
            cf(wk)
        );
    }
};

template <template <typename> class Label,
    template <typename> class Frame, typename RetCont, typename Worker>
struct sfc_label_policy
{
    using derived_type = sfc_label<Label, Frame, RetCont, Worker>;
    using frame_type = sfc_frame<Frame, RetCont, Worker>;
    using user_label_type = Label<sfc_label_base<sfc_label_policy>>;
    
    template <template <typename> class NextLabel>
    using next_label_t = sfc_label<NextLabel, Frame, RetCont, Worker>;
    
    template <template <typename> class NextLabel>
    using next_cont_t = sfc_cont<NextLabel, Frame, RetCont, Worker>;
    
    template <template <typename> class ChildFrame, typename ChildRetCont>
    using child_frame_t = sfc_frame<ChildFrame, ChildRetCont, Worker>;
};


template <template <typename> class Frame, typename Worker, typename... Args>
inline typename sfc_frame<Frame, identity_retcont, Worker>::return_type
call_sfc(Worker& wk, Args&&... args)
{
    sfc_frame<Frame, identity_retcont, Worker> fr(
        mefdn::forward_as_tuple(mefdn::forward<Args>(args)...)
    ,   mefdn::forward_as_tuple()
    );
    
    return fr(wk);
}

} // namespace mefdn
} // namespace menps

