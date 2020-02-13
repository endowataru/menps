
#pragma once

#include <cmpth/fdn.hpp>
#include <cmpth/wrap/atomic_itf_base.hpp>
#include <cmpth/ult_tag.hpp>

namespace cmpth {

class dummy_thread
{
public:
    dummy_thread() noexcept = default;

    template <typename F, typename... Args>
    explicit dummy_thread(F&& f, Args&&... args)
        : joinable_{true}
    {
        fdn::invoke(fdn::forward<F>(f), fdn::forward<Args>(args)...);
    }

    dummy_thread(const dummy_thread&) = delete;
    dummy_thread& operator = (const dummy_thread&) = delete;

    dummy_thread(dummy_thread&& t) noexcept { *this = fdn::move(t); }
    dummy_thread& operator = (dummy_thread&& t) noexcept {
        this->joinable_ = t.joinable_;
        t.joinable_ = false;
        return *this;
    }

    static dummy_thread ptr_fork(void (*func)(void*), void* ptr) {
        return dummy_thread(func, ptr);
    }

    bool joinable() const noexcept { return this->joinable_; }
    void join() {
        this->joinable_ = false;
    }
    void detach() {
        this->joinable_ = false;
    }

private:
    bool joinable_ = false;
};

class dummy_mutex
{
public:
    dummy_mutex() = default;
    
    dummy_mutex(const dummy_mutex&) = delete;
    dummy_mutex& operator = (const dummy_mutex&) = delete;
    
    void lock() { }
    
    bool try_lock() { return true; }
    
    void unlock() { }
};

class dummy_barrier
{
public:
    explicit dummy_barrier(fdn::size_t) { }
    void arrive_and_wait() { }
};

struct dummy_ult_itf
    : atomic_itf_base // TODO: dummy_atomic
{
    using thread = dummy_thread;
    using mutex = dummy_mutex;
    using barrier = dummy_barrier;
    using worker_num_type = fdn::size_t;
    static worker_num_type get_worker_num() noexcept { return 0; }
    static worker_num_type get_num_workers() noexcept { return 1; }
    struct initializer {
        initializer() { }
        explicit initializer(int argc, char** argv) { }
    };
};

template <>
struct get_ult_itf_type<ult_tag_t::DUMMY>
    : fdn::type_identity<dummy_ult_itf> { };

} // namespace cmpth

