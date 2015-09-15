
MGCOM
=====

Communication Library for MassiveThreads/GAS 2.

Requires C++03 or later.

Goals
-----

## General

- Cross-platform.
- Type safety.
- Very thin layer.
    - Easy to read the implementation.
    - Avoid too mush abstraction.

## Communication

- Remote Memory Access (RMA) and Active Messages (AM).
- Non-blocking communication.
- Multithreading support.
    - It can be disabled for single-threaded environments.
- Alignment issues.
- Support for atomic operations.
    - Atomic read/write.
    - Atomic compare-and-swap/fetch-and-op.

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

