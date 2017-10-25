


Components
----------

- mgth
    - DSM + distributed work-stealing scheduler
- mgdsm
    - Distributed Shared Memory (DSM)
- mgcom
    - Portable low-level communication library
- mgdev
    - Communication device wrapper library
- mgult
    - Customizable work-stealing scheduler
- mgctx
    - Context switching library
- mgbase
    - C++ base library

```
+---------------------+
|         mgth        |
+---------+           |
|  mgdsm  |           |
+---------+-----+     |
|      mgcom    |     |
+---------------+     |
|      mgdev    |     |
+---------------+-----+
|        mgult        |
+---------------------+
|        mgctx        |
+---------------------+
|        mgbase       |
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



