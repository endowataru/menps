
MGCOM
=====

Communication Library for MassiveThreads/GAS 2.

This project is experimental and the API is still unstable.

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
Some interconnects have the feature called Remote Direct Memory Access (RDMA).
mgcom provides a thin RMA layer that is specialized for
exploiting the performance of RDMA.

- Support for atomic operations.
    - Atomic read/write.
    - Atomic local compare-and-swap/fetch-and-op.
    - Atomic remote compare-and-swap/fetch-and-op.
- Description of alignment requirements.

### Remote Procedure Call (RPC)

mgcom also provides Remote Procedure Call (RPC)
(a.k.a. Active Messages (AM)).
Currently, only the fixed-size AM is supported
(sending variable-length messages is a future work.)

Platforms
---------

Supported interconnects:

- MPI-1
- MPI-3
- Tofu (FJMPI)
- InfiniBand Verbs

Supported compilers (`-std=c++0x` is required):

- Clang (3.1 or higher)
- GCC (4.4 or higher)
- Intel C++ Compiler (14.0 or higher)


Platform-Specific Information
-----------------------------

### InfiniBand

#### Environment Variables

- `MGCOM_IBV_DEVICE` (default: the first device)
    - Sets the device name.
    - Use the command `ibv_devices` to show all available devices on your platform.
- `MGCOM_IBV_PORT` (default: 1)
    - Sets the port number.
- `MGCOM_IBV_DIRECT` (default: 0)
    - If not "0", communication requests are not offloaded (executed directly).

