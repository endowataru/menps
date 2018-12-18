
#pragma once

#include <menps/medev2/mpi/mpi.hpp>

#define MEDEV2_MPI_P2P_BLOCK_SEND_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        send, \
        Send, \
        int, \
        6, \
        const void *, buf, \
        int, count, \
        MPI_Datatype, datatype, \
        int, dest, \
        int, tag, \
        MPI_Comm, comm) \

#define MEDEV2_MPI_P2P_BLOCK_FUNCS(X, ...) \
    MEDEV2_MPI_P2P_BLOCK_SEND_FUNCS(X, __VA_ARGS__) \
    X(__VA_ARGS__, \
        recv, \
        Recv, \
        int, \
        7, \
        void *, buf, \
        int, count, \
        MPI_Datatype, datatype, \
        int, source, \
        int, tag, \
        MPI_Comm, comm, \
        MPI_Status *, status)

#define MEDEV2_MPI_P2P_NONBLOCK_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        isend, \
        Isend, \
        int, \
        7, \
        const void *, buf, \
        int, count, \
        MPI_Datatype, datatype, \
        int, dest, \
        int, tag, \
        MPI_Comm, comm, \
        MPI_Request *, request) \
    X(__VA_ARGS__, \
        irecv, \
        Irecv, \
        int, \
        7, \
        void *, buf, \
        int, count, \
        MPI_Datatype, datatype, \
        int, source, \
        int, tag, \
        MPI_Comm, comm, \
        MPI_Request *, request) \

#define MEDEV2_MPI_TEST_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        test, \
        Test, \
        int, \
        3, \
        MPI_Request *, request, \
        int *, flag,  \
        MPI_Status *, status) \
    X(__VA_ARGS__,  \
        testsome, \
        Testsome, \
        int, \
        5, \
        int, incount, \
        MPI_Request*, array_of_requests, \
        int*, outcount, \
        int*, array_of_indices, \
        MPI_Status*, array_of_statuses) \
    X(__VA_ARGS__,  \
        wait, \
        Wait, \
        int, \
        2, \
        MPI_Request *, request, \
        MPI_Status *, status)

#define MEDEV2_MPI_PROBE_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        probe, \
        Probe, \
        int, \
        4, \
        int, source, \
        int, tag, \
        MPI_Comm, comm, \
        MPI_Status *, status) \
    X(__VA_ARGS__, \
        iprobe, \
        Iprobe, \
        int, \
        5, \
        int, source, \
        int, tag, \
        MPI_Comm, comm, \
        int *, flag,  \
        MPI_Status *, status)

#define MEDEV2_MPI_COLLECTIVE_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        barrier, \
        Barrier, \
        int, \
        1, \
        MPI_Comm, comm) \
    X(__VA_ARGS__, \
        bcast, \
        Bcast, \
        int, \
        5, \
        void *, buffer, \
        int, count, \
        MPI_Datatype, datatype, \
        int, root,  \
        MPI_Comm, comm) \
    X(__VA_ARGS__, \
        allgather, \
        Allgather, \
        int, \
        7, \
        const void *, sendbuf, \
        int, sendcount, \
        MPI_Datatype, sendtype, \
        void *, recvbuf, \
        int, recvcount, \
        MPI_Datatype, recvtype, \
        MPI_Comm, comm) \
    X(__VA_ARGS__, \
        alltoall, \
        Alltoall, \
        int, \
        7, \
        const void *, sendbuf, \
        int, sendcount, \
        MPI_Datatype, sendtype, \
        void *, recvbuf, \
        int, recvcount, \
        MPI_Datatype, recvtype, \
        MPI_Comm, comm) \
    X(__VA_ARGS__, \
        allreduce, \
        Allreduce, \
        int, \
        6, \
        const void *, sendbuf, \
        void *, recvbuf, \
        int, count, \
        MPI_Datatype, datatype, \
        MPI_Op, op, \
        MPI_Comm, comm)


#define MEDEV2_MPI_RMA_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        get, \
        Get, \
        int, \
        8, \
        void *, origin_addr, \
        int, origin_count, \
        MPI_Datatype, origin_datatype, \
        int, target_rank, \
        MPI_Aint, target_disp, \
        int, target_count, \
        MPI_Datatype, target_datatype, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        put, \
        Put, \
        int, \
        8, \
        const void *, origin_addr, \
        int, origin_count, \
        MPI_Datatype, origin_datatype, \
        int, target_rank, \
        MPI_Aint, target_disp, \
        int, target_count, \
        MPI_Datatype, target_datatype, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        compare_and_swap, \
        Compare_and_swap, \
        int, \
        7, \
        const void *, origin_addr, \
        const void *, compare_addr, \
        void *, result_addr, \
        MPI_Datatype, datatype, \
        int, target_rank, \
        MPI_Aint, target_disp, \
        MPI_Win, win)

#define MEDEV2_MPI_REQ_BASED_RMA_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        rput, \
        Rput, \
        int, \
        9, \
        const void *, origin_addr, \
        int, origin_count, \
        MPI_Datatype, origin_datatype, \
        int, target_rank, \
        MPI_Aint, target_disp, \
        int, target_count, \
        MPI_Datatype, target_datatype, \
        MPI_Win, win, \
        MPI_Request *, request) \
    X(__VA_ARGS__, \
        rget, \
        Rget, \
        int, \
        9, \
        void *, origin_addr, \
        int, origin_count, \
        MPI_Datatype, origin_datatype, \
        int, target_rank, \
        MPI_Aint, target_disp, \
        int, target_count, \
        MPI_Datatype, target_datatype, \
        MPI_Win, win, \
        MPI_Request *, request) \
    X(__VA_ARGS__, \
        rget_accumulate, \
        Rget_accumulate, \
        int, \
        13, \
        const void *, origin_addr, \
        int, origin_count, \
        MPI_Datatype, origin_datatype, \
        void *, result_addr, \
        int, result_count, \
        MPI_Datatype, result_datatype, \
        int, target_rank, \
        MPI_Aint, target_disp, \
        int, target_count, \
        MPI_Datatype, target_datatype, \
        MPI_Op, op, \
        MPI_Win, win, \
        MPI_Request *, request)


#define MEDEV2_MPI_RMA_WIN_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        win_create_dynamic, \
        Win_create_dynamic, \
        int, \
        3, \
        MPI_Info, info, \
        MPI_Comm, comm, \
        MPI_Win *, win) \
    X(__VA_ARGS__, \
        win_free, \
        Win_free, \
        int, \
        1, \
        MPI_Win *, win) \
    X(__VA_ARGS__, \
        win_attach, \
        Win_attach, \
        int, \
        3, \
        MPI_Win, win, \
        void *, base, \
        MPI_Aint, size) \
    X(__VA_ARGS__, \
        win_detach, \
        Win_detach, \
        int, \
        2, \
        MPI_Win, win, \
        const void *, base) \
    X(__VA_ARGS__, \
        win_lock_all, \
        Win_lock_all, \
        int, \
        2, \
        int, assert, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        win_unlock_all, \
        Win_unlock_all, \
        int, \
        1, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        win_flush, \
        Win_flush, \
        int, \
        2, \
        int, rank, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        win_flush_all, \
        Win_flush_all, \
        int, \
        1, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        win_flush_local, \
        Win_flush_local, \
        int, \
        2, \
        int, rank, \
        MPI_Win, win) \
    X(__VA_ARGS__, \
        win_flush_local_all, \
        Win_flush_local_all, \
        int, \
        1, \
        MPI_Win, win)

#define MEDEV2_MPI_OTHER_FUNCS(X, ...) \
    X(__VA_ARGS__, \
        comm_rank, \
        Comm_rank, \
        int, \
        2, \
        MPI_Comm, comm, \
        int *, rank) \
    X(__VA_ARGS__, \
        comm_size, \
        Comm_size, \
        int, \
        2, \
        MPI_Comm, comm, \
        int *, size) \
    X(__VA_ARGS__, \
        comm_dup, \
        Comm_dup, \
        int, \
        2, \
        MPI_Comm, comm, \
        MPI_Comm *, newcomm) \
    X(__VA_ARGS__, \
        get_count , \
        Get_count, \
        int, \
        3, \
        const MPI_Status *, status, \
        MPI_Datatype, datatype, \
        int *, count)

#define MEDEV2_MPI_FUNCS_ALL(X, ...) \
    MEDEV2_MPI_P2P_BLOCK_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_P2P_NONBLOCK_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_TEST_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_PROBE_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_COLLECTIVE_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_RMA_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_REQ_BASED_RMA_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_RMA_WIN_FUNCS(X, __VA_ARGS__) \
    MEDEV2_MPI_OTHER_FUNCS(X, __VA_ARGS__)

namespace menps {
namespace medev2 {
namespace mpi {

#define D(dummy, name, Name, tr, num, ...) \
    struct name##_params { \
        MEDEV2_EXPAND_PARAMS_TO_DECL(num, __VA_ARGS__) \
    };

MEDEV2_MPI_FUNCS_ALL(D, /*dummy*/)

#undef D

} // namespace mpi
} // namespace medev2
} // namespace menps

