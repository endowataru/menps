
#pragma once

#include <cmpth/fdn.hpp>

namespace cmpth {

template <typename P>
class kth_thread
{
    using base_thread_type = typename P::base_thread_type;
    
public:
    kth_thread() noexcept = default;

    template <typename Func, typename... Args>
    explicit kth_thread(Func&& func, Args&&... args)
    {
        using on_fork_type =
            on_fork<fdn::decay_t<Func>, fdn::decay_t<Args>...>;

        this->th_ =
            base_thread_type{
                on_fork_type{
                    fdn::forward<Func>(func)
                ,   fdn::make_tuple(fdn::forward<Args>(args)...)
                }
            };
    }

private:
    template <typename Func, typename... Args>
    struct on_fork {
        Func                func;
        fdn::tuple<Args...> args;

        void operator() () {
            auto& wk_set = P::get_worker_set();
            wk_set.initialize_worker();
            fdn::apply(this->func, this->args);
            wk_set.finalize_worker();
        }
    };

public:
    kth_thread(const kth_thread&&) = delete;
    kth_thread& operator = (const kth_thread&&) = delete;

    kth_thread(kth_thread&&) noexcept = default;
    kth_thread& operator = (kth_thread&&) noexcept = default;

    bool joinable() const noexcept {
        return this->th_.joinable();
    }
    void join() {
        this->th_.join();
    }
    void detach() {
        this->th_.detach();
    }

    static kth_thread ptr_fork(void (*func)(void*), void* ptr) {
        return kth_thread{func, ptr};
    }

private:
    base_thread_type th_;
};

} // namespace cmpth

