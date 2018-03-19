
Components (old figure)
------------------------

- meth
    - DSM + distributed work-stealing scheduler
- medsm
    - Distributed Shared Memory (DSM)
- mecom
    - Portable low-level communication library
- medev
    - Communication device wrapper library
- meult
    - Customizable work-stealing scheduler
- mectx
    - Context switching library
- mefdn
    - C++ base library

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

CMake 3.0 or higher is required.



