
#pragma once

#include <menps/mefdn/coro/coro.hpp>
#include <menps/mefdn/arithmetic.hpp>

namespace menps {
namespace mefdn {

template <template <typename> class Frame, typename RetCont, typename Worker>
class slc_frame;

template <template <typename> class Label,
    template <typename> class Frame, typename RetCont, typename Worker>
struct slc_label_policy;

template <template <typename> class Label,
    template <typename> class Frame, typename RetCont, typename Worker>
using slc_label = coro_label<slc_label_policy<Label, Frame, RetCont, Worker>>;

// TODO: Remove this strange class...
//       It is necessary to get the maximum size for children frames.
//       Those sizes are depending on those of their return continuations.
class slc_dummy_ret_cont { };

template <typename Worker, template <typename> class... ChildFrames>
struct slc_children
{
public:
    template <template <typename> class ChildFrame, typename RetCont, typename... Args>
    slc_frame<ChildFrame, RetCont, Worker>& construct_child_frame(Args&&... args) {
        using frame_type = slc_frame<ChildFrame, RetCont, Worker>;
        MEFDN_STATIC_ASSERT(sizeof(frame_type) <= buf_size);
        
        return * new (buf) frame_type(
            mefdn::forward<Args>(args)...
        );
    }

    template <template <typename> class ChildFrame, typename RetCont>
    static slc_children& destruct_child_frame(slc_frame<ChildFrame, RetCont, Worker>&& cf) {
        using frame_type = slc_frame<ChildFrame, RetCont, Worker>;
        MEFDN_STATIC_ASSERT(sizeof(frame_type) <= buf_size);
        
        cf.~slc_frame();
        // TODO: manipulate offset of buf
        return reinterpret_cast<slc_children&>(cf);
    }

private:
    static constexpr const mefdn::size_t buf_size =
        mefdn::static_max<mefdn::size_t,
            sizeof(slc_frame<ChildFrames, slc_dummy_ret_cont, Worker>)...>::value;
    
    mefdn::byte buf[buf_size];
};

template <typename Worker>
struct slc_children<Worker> { };

template <typename Worker>
class slc_frame_base
{
public:
    template <template <typename> class... ChildFrames>
    using define_children = slc_children<Worker, ChildFrames...>;
    
    using children = define_children<>;
    
    template <typename... Results>
    using task = typename Worker::template task<Results...>;
    
    using worker = Worker;
};

template <template <typename> class Frame, typename RetCont, typename Worker>
struct slc_frame_policy
{
    using derived_type = slc_frame<Frame, RetCont, Worker>;
    using user_frame_type = Frame<slc_frame_base<Worker>>;
    using ret_cont_type = RetCont;
    using worker_type = Worker;
    using start_label_type =
        slc_label<user_frame_type::template start, Frame, RetCont, Worker>;
};

template <template <typename> class Frame, typename RetCont, typename Worker>
class slc_frame
    : public basic_coro_frame<slc_frame_policy<Frame, RetCont, Worker>>
    , public Frame<slc_frame_base<Worker>>::children
{
    using base = basic_coro_frame<slc_frame_policy<Frame, RetCont, Worker>>;
    
public:
    using base::base;
};



template <template <typename> class ContLabel,
    template <typename> class Frame, typename RetCont, typename Worker>
class slc_cont
{
    using frame_type = slc_frame<Frame, RetCont, Worker>;
    using return_type = typename frame_type::return_type;
    
public:
    template <typename ChildLabel, typename... Args>
    return_type operator() (ChildLabel&& ch_lb, Args&&... args) {
        auto& wk = ch_lb.get_worker();
        
        // Destruct the child frame.
        auto& ch_fr = ch_lb.get_frame();
        auto& ch_frs = frame_type::destruct_child_frame(mefdn::move(ch_fr));
        auto& fr = static_cast<frame_type&>(ch_frs);
        
        // Create the label for the continuation.
        slc_label<ContLabel, Frame, RetCont, Worker> lb(fr, wk);
        return lb(mefdn::forward<Args>(args)...);
    }
};

template <typename P>
class slc_label_base
    : public basic_coro_label<P>
{
    MEFDN_DEFINE_DERIVED(P)
    
    using base = basic_coro_label<P>;
    
public:
    using typename base::worker_type;
    using typename base::return_type;
    
public:
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame, typename... Args>
    return_type call(Args&&... args)
    {
        auto& self = this->derived();
        auto& fr = self.get_frame();
        auto& wk = self.get_worker();
        
        using ret_cont_type = typename P::template next_cont_t<NextLabel>;
        
        auto& cf =
            fr.template construct_child_frame<ChildFrame, ret_cont_type>(
                mefdn::forward_as_tuple(mefdn::forward<Args>(args)...)
            ,   mefdn::forward_as_tuple()
            );
        
        return cf(wk);
    }
};

template <template <typename> class Label,
    template <typename> class Frame, typename RetCont, typename Worker>
struct slc_label_policy
{
    using derived_type = slc_label<Label, Frame, RetCont, Worker>;
    using frame_type = slc_frame<Frame, RetCont, Worker>;
    using user_label_type = Label<slc_label_base<slc_label_policy>>;
    
    template <template <typename> class NextLabel>
    using next_label_t = slc_label<NextLabel, Frame, RetCont, Worker>;
    
    template <template <typename> class NextLabel>
    using next_cont_t = slc_cont<NextLabel, Frame, RetCont, Worker>;
    
    template <template <typename> class ChildFrame, typename ChildRetCont>
    using child_frame_t = slc_frame<ChildFrame, ChildRetCont, Worker>;
};

} // namespace mefdn
} // namespace menps

