
MENPS
=====

MENPS is a distributed shared memory library designed for high-performance computing.
MENPS means that "<b>ME</b>NPS is <b>N</b>ot a <b>P</b>GAS <b>S</b>ystem".

Components
----------

This repository is composed of multiple modules for distributed-memory programming.

- meomp
    - OpenMP implementation on DSM
- medsm2
    - Distributed Shared Memory (DSM)
- mecom2
    - Portable low-level communication library
- meqdc
    - <b>Q</b>ueue-<b>d</b>elegated <b>c</b>ommunication device wrapper
- medev2
    - Communication device wrapper library
- meult
    - Customizable work-stealing scheduler
- mectx
    - Context switching library
- mefdn
    - C++ base library
- cmpth
    - ComposableThreads, a standalone user-level threading library

```
+---------+
|  meomp  |
+---------+
|  medsm2 |
+---------+--+
|    mecom2  |
+------------+
|    meqdc   |
+------------+
|    medev2  |
+---------------+-----+
|        meult        | -> will be replaced with cmpth
+---------------------+
|        mectx        | -> will be replaced with cmpth
+---------------------+
|        mefdn        |
+---------------------+
```

Requirements
------------

- GCC 4.8 or later
- CMake 3.1 or later.

How to Build
------------

```
git clone --recursive [url] [directory]
cd [directory]
./cmake.bash Debug
./make.bash Debug
```

Executable files will be generated in `build/`.


