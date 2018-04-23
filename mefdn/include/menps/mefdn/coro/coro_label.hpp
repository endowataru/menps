
#pragma once

#include <menps/mefdn/utility.hpp>
#include <menps/mefdn/tuple.hpp>
#include <menps/mefdn/type_traits/integer_sequence.hpp>

namespace menps {
namespace mefdn {

template <typename P>
class basic_coro_label
{
    MEFDN_DEFINE_DERIVED(P)
    
public:
    using frame_type = typename P::frame_type;
    using worker_type = typename frame_type::worker_type;
    using return_type = typename frame_type::return_type;
    
    template <template <typename> class ChildFrame, typename ChildRetCont>
    using child_frame_t = typename P::template child_frame_t<ChildFrame, ChildRetCont>;
    
    frame_type& frame() {
        auto& self = this->derived();
        return self.get_frame();
    }
    worker_type& worker() {
        auto& self = this->derived();
        return self.get_worker();
    }
    
    template <template <typename> class NextLabel, typename... Args>
    return_type jump(Args&&... args) {
        auto& self = this->derived();
        auto lb = self.template make_next_label<NextLabel>();
        return lb(mefdn::forward<Args>(args)...);
    }
    
    template <template <typename> class NextLabel, typename... Args>
    return_type jump_with_tuple(mefdn::tuple<Args...> t) {
        return this->template jump_with_tuple<NextLabel>(
            mefdn::move(t)
        ,   mefdn::index_sequence_for<Args...>()
        );
    }
    
private:
    template <template <typename> class NextLabel, typename... Args, mefdn::size_t... Ns>
    return_type jump_with_tuple(
        mefdn::tuple<Args...> t
    ,   mefdn::index_sequence<Ns...> /*ignored*/
    ) {
        return this->template jump<NextLabel>( mefdn::get<Ns>(t)... );
    }
    
public:
    template <typename... Args>
    return_type set_return(Args&&... args) {
        auto& self = this->derived();
        auto& fr = this->frame();
        return fr.set_return(
            mefdn::move(self) // This label is moved.
        ,   mefdn::forward<Args>(args)...
        );
    }
    
    template <template <typename> class NextLabel, typename Func, typename... Args>
    return_type suspend(Func&& func, Args&&... args) {
        auto& self = this->derived();
        return self.get_worker().suspend(
            self.get_frame()
        ,   self.template make_next_label<NextLabel>()
        ,   mefdn::forward<Func>(func)
        ,   mefdn::forward<Args>(args)...
        );
    }
    
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame,
        typename Task, typename... Args>
    return_type fork(
        Task&       tk
    ,   Args&&...   args
    ) {
        auto& self = this->derived();
        return self.get_worker()
            .template fork<NextLabel, ChildFrame>(
                self
            ,   tk
            ,   mefdn::forward<Args>(args)...
            );
    }
    
    template <template <typename> class NextLabel,
        template <typename> class ChildFrame,
        typename Task, typename... Args>
    return_type fork_rec(
        Task&       tk
    ,   Args&&...   args
    ) {
        auto& self = this->derived();
        return self.get_worker()
            .template fork_rec<NextLabel, ChildFrame>(
                self
            ,   tk
            ,   mefdn::forward<Args>(args)...
            );
    }
    
    template <template <typename> class NextLabel, typename Task>
    return_type join(Task& tk) {
        auto& self = this->derived();
        return self.get_worker()
            .template join<NextLabel>(self, tk);
    }
    
    template <template <typename> class NextLabel>
    return_type yield() {
        auto& self = this->derived();
        return self.get_worker()
            .template yield<NextLabel>(self);
    }
    
    template <template <typename> class NextLabel, typename Mutex>
    return_type lock(Mutex& mtx) {
        auto& self = this->derived();
        return self.get_worker().lock(
            self.get_frame()
        ,   self.template make_next_label<NextLabel>()
        ,   mtx
        );
    }
    
    template <template <typename> class NextLabel, typename Mutex>
    return_type unlock(Mutex& mtx) {
        auto& self = this->derived();
        return self.get_worker().unlock(
            self.get_frame()
        ,   self.template make_next_label<NextLabel>()
        ,   mtx
        );
    }
};

template <typename P>
class coro_label
    : public P::user_label_type
{
    using base = typename P::user_label_type;
    
    using frame_type = typename P::frame_type;
    using worker_type = typename frame_type::worker_type;
    
public:
    explicit coro_label(frame_type& fr, worker_type& wk)
        : fr_(fr)
        , wk_(wk)
    { }
    
    frame_type& get_frame() const noexcept {
        return this->fr_;
    }
    worker_type& get_worker() const noexcept {
        return this->wk_;
    }
    
    template <template <typename> class NextLabel>
    typename P::template next_label_t<NextLabel> make_next_label() {
        return typename P::template next_label_t<NextLabel>(
            this->fr_, this->wk_);
    }
    
private:
    frame_type& fr_;
    worker_type& wk_;
};

} // namespace mefdn
} // namespace menps

