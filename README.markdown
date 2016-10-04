
Components
----------

- mgth
    - DSM + work-stealing scheduler
- mgdsm
    - Distributed Shared Memory (DSM)
- mgas2
    - Partitioned Global Address Space (PGAS) library with dynamic page migration
- mgcom
    - Low-level communication library
- mgult
    - Work-stealing scheduler
- mgbase
    - C++ base library

```
+-----------------------+
|         mgth          |
+-----------+-+---------+
|   mgdsm   | |         |
+-----------+ |         |
|   mgas2   | |  mgult  |
+-----------+ |         |
|   mgcom   | |         |
+-----------+-+---------+
|         mgbase        |
+-----------------------+
```


How to Build
------------

```
git clone --recursive [url] [directory]
cd [directory]
./cmake.sh Debug
./make.sh Debug
```

CMake 3.0 or higher is required.

