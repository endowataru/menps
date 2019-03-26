
MENPS
=====

MENPS is a distributed shared memory library designed for high-performance computing.
The term MENPS is an acronym of "__ME__nps is __N__ot a __P__GAS __S__ystem".

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
    - __Q__ueue-__d__elegated __c__ommunication device wrapper
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

Here are some old modules that are not maintained any more.

- meth
    - DSM + distributed work-stealing scheduler
- medsm
    - Distributed Shared Memory (DSM)
- mecom
    - Portable low-level communication library
- medev
    - Communication device wrapper library

This is an old figure that describes the dependencies.

```
+---------------------+
|         meth        |
+---------+           |
|  medsm  |           |
+---------+-----+     |
|      mecom    |     |
+---------------+     |
|      medev    |     |
+---------------+-----+
|        meult        |
+---------------------+
|        mectx        |
+---------------------+
|        mefdn        |
+---------------------+
```

Requirements
------------

- GCC 4.8 or later
- CMake 3.0 or later.

How to Build
------------

```
git clone --recursive [url] [directory]
cd [directory]
./cmake.sh Debug
./make.sh Debug
```

Executable files will be generated in `build/`.


