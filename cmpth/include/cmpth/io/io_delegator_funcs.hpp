
#pragma once

#include <cmpth/fdn.hpp>

#include <sys/types.h>
#include <sys/socket.h>

namespace cmpth {

#define CMPTH_IO_BYPASS_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        listen, \
        int, \
        2, \
        int, sockfd, \
        int, backlog) \
    X(__VA_ARGS__, \
        bind, \
        int, \
        3, \
        int, sockfd, \
        const struct sockaddr*, addr, \
        socklen_t, addrlen) \
    X(__VA_ARGS__, \
        connect, \
        int, \
        3, \
        int, sockfd, \
        const struct sockaddr *, addr, \
        socklen_t, addrlen) \
    X(__VA_ARGS__, \
        select, \
        int, \
        5, \
        int, nfds, \
        fd_set *, readfds, \
        fd_set *, writefds, \
        fd_set *, exceptfds, \
        struct timeval *, timeout)

#define CMPTH_IO_SETUP_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        socket, \
        int, \
        3, \
        int, domain, \
        int, type, \
        int, protocol) \
    X(__VA_ARGS__, \
        close, \
        int, \
        1, \
        int, fd)

#define CMPTH_IO_READ_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        accept, \
        int, \
        3, \
        int, sockfd, \
        sockaddr*, addr, \
        socklen_t*, addr_len) \
    X(__VA_ARGS__, \
        recv, \
        ssize_t, \
        4, \
        int, sockfd, \
        void*, buf, \
        size_t, n, \
        int, flags) \
    X(__VA_ARGS__, \
        recvfrom, \
        ssize_t, \
        6, \
        int, sockfd, \
        void*, buf, \
        size_t, len, \
        int, flags, \
        sockaddr *, src_addr, \
        socklen_t*, addrlen)

#define CMPTH_IO_WRITE_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        send, \
        ssize_t, \
        4, \
        int, sockfd, \
        const void*, buf, \
        size_t, n, \
        int, flags) \
    X(__VA_ARGS__, \
        sendto, \
        ssize_t, \
        6, \
        int, sockfd, \
        const void*, buf, \
        size_t, len, \
        int, flags, \
        const sockaddr*, src_addr, \
        socklen_t, addrlen)

#define CMPTH_IO_DELEGATED_FUNCS(X, ...) \
    CMPTH_IO_READ_FUNCS(X, __VA_ARGS__) \
    CMPTH_IO_WRITE_FUNCS(X, __VA_ARGS__)

#define CMPTH_IO_ALL_FUNCS(X, ...) \
    CMPTH_IO_BYPASS_FUNCS(X, __VA_ARGS__) \
    CMPTH_IO_SETUP_FUNCS(X, __VA_ARGS__) \
    CMPTH_IO_DELEGATED_FUNCS(X, __VA_ARGS__)


enum io_operation_kind {
    invalid = 0
    #define D(dummy, name, tr, num, ...)    , name
    CMPTH_IO_DELEGATED_FUNCS(D, /*dummy*/)
    #undef D
};

struct io_operation_result {
    fdn::int64_t ret;
    int err;
};

#define D(dummy, name, tr, num, ...) \
    struct io_##name##_params { \
        CMPTH_EXPAND_PARAMS_TO_DECL(num, __VA_ARGS__) \
    };

CMPTH_IO_ALL_FUNCS(D, /*dummy*/)

#undef D

} // namespace cmpth

