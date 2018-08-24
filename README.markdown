
MENPS
=====

MENPS is a distributed shared memory library for high-performance computing.
The term MENPS is an acronym of "MEnps is Not a Pgas System".

Components
----------

This repository is composed of multiple modules for distributed-memory programming.

- medsm2
    - Distributed Shared Memory (DSM)
- mecom2
    - Portable low-level communication library
- medev
    - Communication device wrapper library
- meult
    - Customizable work-stealing scheduler
- mectx
    - Context switching library
- mefdn
    - C++ base library

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


