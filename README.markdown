
MGCOM
=====

Communication Library for MassiveThreads/GAS 2.

Requires C++03 or later.

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
- Alignment issues.
- Support for atomic operations.
    - Atomic read/write.
    - Atomic compare-and-swap/fetch-and-op.

Features
--------

### Remote Memory Access (RMA)

RMA (a.k.a. one-sided communication)
enables users to read/write the memory on the remote nodes over interconnects.
Some interconnect hardwares have the feature called Remote Direct Memory Access (RDMA).
mgcom provides a thin RMA layer that is specialized for
exploiting the performance of RDMA.

### Active Messages (AM)

mgcom also provides Active Messages (AM),
which is one representation of Remote Procedure Call (RPC).
Currently, only the fixed-size AM is supported
(sending variable-length message is a future work.)

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

