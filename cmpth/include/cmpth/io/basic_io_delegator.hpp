
#pragma once

#include <cmpth/io/io_epoll_object.hpp>
#include <cmpth/io/io_delegator_funcs.hpp>

#include <vector>
#include <unordered_map>

#include <fcntl.h>

namespace cmpth {

template <typename P>
class basic_io_consumer
{
    using suspended_thread_type = typename P::suspended_thread_type;

public:
    suspended_thread_type* create_pending(const bool is_write, const int fd) {
        auto& fd_ent = this->get_fd_entry(fd);
        if (is_write) {
            if (fd_ent.wr_ready) {
                fd_ent.wr_ready = false;
                return nullptr;
            }
            else {
                ++this->num_ongoing_;
                fd_ent.wr_pendings.emplace_back();
                return &fd_ent.wr_pendings.back();
            }
        }
        else {
            if (fd_ent.rd_ready) {
                fd_ent.rd_ready = false;
                return nullptr;
            }
            else {
                ++this->num_ongoing_;
                fd_ent.rd_pendings.emplace_back();
                return &fd_ent.rd_pendings.back();
            }
        }
    }

    struct delegated_func_type {
        bool                    is_write;
        int                     fd;
        suspended_thread_type   wait_sth;
    };

    fdn::tuple<bool, suspended_thread_type> execute(delegated_func_type& df) {
        if (auto* const pending_sth = this->create_pending(df.is_write, df.fd)) {
            *pending_sth = fdn::move(df.wait_sth);
            return fdn::make_tuple(true, suspended_thread_type());
        }
        else {
            return fdn::make_tuple(true, fdn::move(df.wait_sth));
        }
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
                    CMPTH_P_LOG_DEBUG(P, "Wake up read I/O requests.");
                    this->flush_suspended(fd_ent.rd_ready, fd_ent.rd_pendings, ret_sth);
                }
                if ((ef & EPOLLOUT) != 0) {
                    CMPTH_P_LOG_DEBUG(P, "Wake up write I/O requests.");
                    this->flush_suspended(fd_ent.wr_ready, fd_ent.wr_pendings, ret_sth);
                }
            }
        }
        return ret_sth;
    }

    bool is_active() const noexcept {
        return this->num_ongoing_ > 0;
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
    using pending_list_type = std::vector<suspended_thread_type>;

    void flush_suspended(bool& ready, pending_list_type& pendings, suspended_thread_type& ret_sth) {
        if (pendings.empty()) {
            ready = true;
            return;
        }
        this->num_ongoing_ -= pendings.size();
        for (auto& sth: pendings) {
            if (ret_sth) { sth.notify(); }
            else { ret_sth = fdn::move(sth); }
        }
        pendings.clear();
    }

    struct fd_entry {
        bool rd_ready = false;
        bool wr_ready = false;
        pending_list_type rd_pendings;
        pending_list_type wr_pendings;
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

    #define D(is_write_, name, tr, num, ...) \
        tr execute_##name(const io_##name##_params& p) { \
            CMPTH_P_LOG_DEBUG(P \
            ,   "Entering " #name "() on producer thread." \
            ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
            ); \
            while (true) { \
                const tr ret = ::name(CMPTH_EXPAND_PARAMS_TO_P_DOT_ARGS(num, __VA_ARGS__)); \
                const auto e = errno; \
                if (!(ret == -1 && (e == EAGAIN || e == EWOULDBLOCK))) { \
                    CMPTH_P_LOG_DEBUG(P \
                    ,   "Exiting " #name " on producer thread." \
                    ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                    ); \
                    return ret; \
                } \
                CMPTH_P_LOG_DEBUG(P \
                ,   "Start blocking " #name "() on producer thread." \
                ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                ); \
                del_.execute_or_delegate( \
                    [this, &p] { \
                        return fdn::make_tuple(true, this->con_.create_pending(is_write_, p.sockfd)); \
                    } \
                ,   [&p] (delegated_func_type& df) { \
                        df.is_write = is_write_; \
                        df.fd = p.sockfd; \
                        return &df.wait_sth; \
                    } \
                ); \
                CMPTH_P_LOG_DEBUG(P \
                ,   "Finish blocking " #name "() on producer thread." \
                ,   CMPTH_EXPAND_PARAMS_TO_LOG_P_DOT(num, __VA_ARGS__) \
                ); \
            } \
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

