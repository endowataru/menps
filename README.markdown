
MGCOM
=====

Communication Library for MassiveThreads/GAS 2.

This project is experimental and the API is still unstable.

Requires C++03 or later.

Description
-----------

"mgcom" is designed to develop middleware
running on distributed memory environments
with tightly-coupled interconnect hardware.
Although the initial motivation was
to develop a Partitioned Global Address Space (PGAS) 
and a distributed task scheduler,
you can utilize it
as a general communication library
instead of current existing communication libraries
(e.g, MPI, GASNet.)

Because "mgcom" is a low-layer communication library,
any intrinsic overheads caused by hardware issues
are exposed to the mgcom's users to gain the performance.
Some burdensome operations such as registration of memory regions
and allocating temporary buffers are required to be manually done by the users.

"mgcom" provides a statically-typed interface in C++
to improve the productivity of middleware.
It is useful to create and operate
a complex data structure in distributed memory systems.
It doesn't incur any additional overheads
because it's just an thin wrapper of pointer arithmetics in C.

Goals
-----

### General

- Cross-platform.
- Type safety.
- Very thin layer.
    - Easy to read the implementation.
    - Avoid too mush abstraction.

### Communication

- Remote Memory Access (RMA) and Active Messages (AM).
- Non-blocking communication.
- Multithreading support.
    - It can be disabled for single-threaded environments.

Features
--------

### Remote Memory Access (RMA)

RMA (a.k.a. one-sided communication)
enables users to read/write the memory on the remote nodes over interconnects.
Some interconnect hardwares have the feature called Remote Direct Memory Access (RDMA).
mgcom provides a thin RMA layer that is specialized for
exploiting the performance of RDMA.

- Support for atomic operations.
    - Atomic read/write.
    - Atomic local compare-and-swap/fetch-and-op.
    - Atomic remote compare-and-swap/fetch-and-op.
- Description of alignment requirements.

### Active Messages (AM)

mgcom also provides Active Messages (AM),
which is one representation of Remote Procedure Call (RPC).
Currently, only the fixed-size AM is supported
(sending variable-length messages is a future work.)

Platforms
---------

The platforms to be supported are:

- MPI-3
- Fujitsu RDMA
    - Fujitsu compiler
    - GCC
- Infiniband Verbs


How to Build & Run
------------------

    git clone --recursive git@gitlab.eidos.ic.i.u-tokyo.ac.jp:wendo/mgcom.git
    cd mgcom
    ./build
    make latency
    mpirun -n 2 ./out/Release/latency

