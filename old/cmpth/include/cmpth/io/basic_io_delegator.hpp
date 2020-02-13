
#pragma once

#include <cmpth/io/io_epoll_object.hpp>
#include <cmpth/io/io_delegator_funcs.hpp>

#include <queue>
#include <unordered_map>

#include <fcntl.h>

namespace cmpth {

template <typename P>
class basic_io_consumer
{
    using suspended_thread_type = typename P::suspended_thread_type;

public:
    struct delegated_func_type {
        io_operation_kind kind;
        union {
            #define D(dummy, name, tr, num, ...) io_##name##_params name;
            CMPTH_IO_DELEGATED_FUNCS(D, /*dummy*/)
            #undef D
        }
        params;
        io_operation_result* out_res;
        suspended_thread_type wait_sth;
    };

    fdn::tuple<bool, suspended_thread_type> execute(delegated_func_type& df) {
        auto ret = this->execute(df, true);
        fdn::get<0>(ret) = true; // TODO
        return ret;
    }

    suspended_thread_type progress() {
        suspended_thread_type ret_sth;
        struct epoll_event events[P::num_poll_events];
        CMPTH_P_LOG_VERBOSE(P,
            "Entering epoll_wait()."
        ,   "epfd", this->eo_.get_epfd()
        );
        const auto poll_ret = this->eo_.poll(events, P::num_poll_events);
        CMPTH_P_LOG_VERBOSE(P,
            "Exiting epoll_wait()."
        ,   "epfd", this->eo_.get_epfd()
        ,   "ret", poll_ret
        );
        if (poll_ret == 0) {
            P::yield();
        }
        else {
            for (int i = 0; i < poll_ret; ++i) {
                CMPTH_P_LOG_DEBUG(P, "Try to wake up requests.");
                const auto ef = events[i].events;
                auto& fd_ent = *static_cast<fd_entry*>(events[i].data.ptr);
                if ((ef & EPOLLIN) != 0) {
                    CMPTH_P_LOG_DEBUG(P, "Try to wake up read I/O requests.");
                    progress_execute_until_failure(fd_ent.rd_pending, ret_sth);
                }
                if ((ef & EPOLLOUT) != 0) {
                    CMPTH_P_LOG_DEBUG(P, "Try to wake up write I/O requests.");
                    progress_execute_until_failure(fd_ent.wr_pending, ret_sth);
                }
            }
        }
        return ret_sth;
    }

    bool is_active() const noexcept {
        return this->num_ongoing_ > 0;
    }
    
    delegated_func_type& create_pending(const bool is_write, const int fd) {
        ++this->num_ongoing_;
        auto& fd_ent = this->get_fd_entry(fd);
        auto& pending = is_write ? fd_ent.wr_pending : fd_ent.rd_pending;
        pending.emplace();
        return pending.back();
    }

    void add_fd(const int fd) {
        const auto r = this->fd_map_.emplace(fd, fd_entry{});
        fd_entry& fd_ent = r.first->second;
        this->eo_.add(fd, EPOLLIN|EPOLLOUT|EPOLLRDHUP|EPOLLET, &fd_ent);
    }
    void remove_fd(const int fd) {
        this->fd_map_.erase(fd);
        this->eo_.remove(fd);
    }

private:
    void progress_execute_until_failure(
        std::queue<delegated_func_type>&    pending
    ,   suspended_thread_type&              ret_sth
    ) {
        while (!pending.empty()) {
            auto& df = pending.front();
            auto r = this->execute(df, false);
            if (!fdn::get<0>(r)) {
                break;
            }
            auto& sth = fdn::get<1>(r);
            if (ret_sth) { ret_sth = fdn::move(sth); }
            else { sth.notify(); }

            pending.pop();
            CMPTH_P_ASSERT(P, this->num_ongoing_ > 0);
            --this->num_ongoing_;
        }
    }
    
    fdn::tuple<bool, suspended_thread_type>
    execute(delegated_func_type& df, const bool put_pending) {
        switch (df.kind) {
            #define D(is_write, name, tr, num, ...) \
                case io_operation_kind::name: { \
                    auto& p = df.params.name; \
                    CMPTH_P_LOG_DEBUG(P \
                    ,   "Entering " #name " on consumer thread." \
                    ,   "num_ongoing", this->num_ongoing_ \
                    ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                    ); \
                    const int ret = \
                        ::name(CMPTH_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__)); \
                    const auto e = errno; \
                    if (ret == -1 && (e == EAGAIN || e == EWOULDBLOCK)) { \
                        /* Could not execute. Do it again later. */ \
                        if (put_pending) { \
                            auto& new_df = this->create_pending(is_write, p.sockfd); \
                            new_df = fdn::move(df); \
                        } \
                        auto& p = df.params.name; \
                        CMPTH_P_LOG_DEBUG(P \
                        ,   "Failed " #name " on consumer thread." \
                        ,   "num_ongoing", this->num_ongoing_ \
                        ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                        ); \
                        return fdn::make_tuple(false, suspended_thread_type()); \
                    } \
                    else { \
                        /* (1) Success or (2) unhandled error. */ \
                        /* Notify the original thread. */ \
                        auto& res = *df.out_res; \
                        res.ret = ret; \
                        res.err = e; \
                        auto& p = df.params.name; \
                        CMPTH_P_LOG_DEBUG(P \
                        ,   "Succeeded " #name " on consumer thread." \
                        ,   "num_ongoing", this->num_ongoing_ \
                        ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                        ); \
                        CMPTH_P_ASSERT(P, df.wait_sth); \
                        return fdn::make_tuple(true, fdn::move(df.wait_sth)); \
                    } \
                    break; \
                }
            CMPTH_IO_READ_FUNCS(D, false)
            CMPTH_IO_WRITE_FUNCS(D, true)
            #undef D
        }
        std::abort();
    }

    struct fd_entry {
        std::queue<delegated_func_type> rd_pending;
        std::queue<delegated_func_type> wr_pending;
    };
    fd_entry& get_fd_entry(int fd) {
        CMPTH_P_ASSERT(P, this->fd_map_.find(fd) != fdn::end(this->fd_map_));
        return this->fd_map_[fd];
    }

    io_epoll_object eo_;
    std::unordered_map<int, fd_entry> fd_map_;
    fdn::size_t num_ongoing_ = 0;
};

template <typename P>
class basic_io_producer
{
    using delegator_type = typename P::delegator_type;
    using consumer_type = typename delegator_type::consumer_type;
    using delegated_func_type = typename consumer_type::delegated_func_type;

public:
    basic_io_producer() {
        del_.start_consumer(con_);
    }
    ~basic_io_producer() {
        del_.stop_consumer();
    }

    #define D(is_write, name, tr, num, ...) \
        tr execute_##name(const io_##name##_params& p) { \
            CMPTH_P_LOG_DEBUG(P \
            ,   "Entering " #name " on producer thread." \
            ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
            ); \
            io_operation_result res = io_operation_result(); \
            const bool locked = this->del_.lock_or_delegate( \
                [&p, &res] (delegated_func_type& df) { \
                    df.kind = io_operation_kind::name; \
                    df.params.name = p; \
                    df.out_res = &res; \
                    return &df.wait_sth; \
                } \
            ); \
            if (locked) { \
                CMPTH_P_LOG_DEBUG(P \
                ,   "Executing " #name " on producer thread." \
                ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                ); \
                const tr ret = ::name(CMPTH_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__)); \
                const auto e = errno; \
                if (ret == -1 && (e == EAGAIN || e == EWOULDBLOCK)) { \
                    CMPTH_P_LOG_DEBUG(P \
                    ,   "Start blocking " #name " on producer thread." \
                    ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                    ); \
                    auto& df = this->con_.create_pending(is_write, p.sockfd); \
                    df.kind = io_operation_kind::name; \
                    df.params.name = p; \
                    df.out_res = &res; \
                    this->del_.unlock_and_wait(df.wait_sth); \
                    CMPTH_P_LOG_DEBUG(P \
                    ,   "Finish blocking " #name " on producer thread." \
                    ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                    ); \
                } \
                else { \
                    this->del_.unlock(); \
                    CMPTH_P_LOG_DEBUG(P \
                    ,   "Returning " #name " on producer thread." \
                    ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                    ); \
                    return ret; \
                } \
            } \
            CMPTH_P_LOG_DEBUG(P \
            ,   "Returning " #name " on producer thread." \
            ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
            ); \
            errno = res.err; \
            return static_cast<tr>(res.ret); \
        }

    CMPTH_IO_READ_FUNCS(D, false)
    CMPTH_IO_WRITE_FUNCS(D, true)

    #undef D

    int execute_socket(const io_socket_params& p) {
        this->del_.lock();
        const int ret = socket(p.domain, p.type | O_NONBLOCK, p.protocol);
        if (ret != -1) {
            this->con_.add_fd(ret);
        }
        this->del_.unlock();
        return ret;
    }

    int execute_close(const io_close_params& p) {
        this->del_.lock();
        const int ret = close(p.fd);
        this->con_.remove_fd(p.fd);
        this->del_.unlock();
        return ret;
    }

private:
    consumer_type con_;
    delegator_type del_;
};

} // namespace cmpth

