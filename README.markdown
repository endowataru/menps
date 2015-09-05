
MGCOM
=====

Communication Library for MassiveThreads/GAS 2.

Requires C++03 or later.

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

