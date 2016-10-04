
Components
----------

- mgbase
    - Base library for C++
- mgcom
    - Low-level communication library
- mgas2
    - Partitioned Global Address Space (PGAS) library with dynamic page migration
- mgdsm
    - Distributed Shared Memory (DSM)
- mgult
    - Work-stealing scheduler
- mgth
    - DSM + work-stealing scheduler

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

