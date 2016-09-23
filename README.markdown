
Components
----------

- mgbase
    - Base library for C++
- mgcom
    - Low-layer communication library
- mgas2
    - Partitioned Global Address Space (PGAS) library with dynamic page migration

```
+-------------+
|   mgas2     |
+-------------+
|   mgcom     |
+-------------+
|   mgbase    |
+-------------+
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

