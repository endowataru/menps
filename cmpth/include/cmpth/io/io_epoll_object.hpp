
#pragma once

#include <cmpth/fdn.hpp>

#include <unistd.h>
#include <sys/epoll.h>

namespace cmpth {

class io_epoll_object {
public:
    io_epoll_object() {
        this->epfd_ = epoll_create1(0);
    }
    ~io_epoll_object() {
        close(this->epfd_);
    }

    io_epoll_object(const io_epoll_object&) = delete;
    io_epoll_object& operator = (const io_epoll_object&) = delete;

    void add(const int fd, const int event_set, void* const ptr) {
        epoll_event ee = epoll_event();
        ee.events = event_set;
        ee.data.ptr = ptr;
        epoll_ctl(this->epfd_, EPOLL_CTL_ADD, fd, &ee);
    }
    void remove(const int fd) {
        epoll_event ee = epoll_event();
        epoll_ctl(this->epfd_, EPOLL_CTL_DEL, fd, &ee);
    }

    int poll(epoll_event* const events, const int num_events) {
        const int ret = epoll_wait(this->epfd_, events, num_events, 0);
        return ret;
    }

    int get_epfd() const noexcept {
        return this->epfd_;
    }

private:
    int epfd_ = -1;
};

} // namespace cmpth

