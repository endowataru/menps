
# ComposableThreads

A composable, policy-based user-level thread library.

## Design decisions

- Header-only.
    - No prior compilation is needed for the users.
    - All of the configurations are given as policy classes.
- Compatible interface with C++11 threading.
- Static type checking.
    - Leveraging the power of generic programming,
      dynamic function invocations are minimized
      while providing flexible components in each layer.
    - Every layer ensures type safety to reduce the bugs.

## Features

- Context switching.
- Work-stealing scheduler.
- Locking.
    - MCS spinlocks/mutexes.
    - MCS lock delegator.
- Multi-threaded object pool.
    - Free from resource exhaustion due to thread migrations.

## Why I need a new library

Existing ULT libraries are NOT composable.
That is, they all try to be "complete" libraries for general purposes.
They usually force the library users to use the single API & implementation
(typically in plain C API & implementation) regardless of their usage and requirements.
However, what I really need is to tweak the individual features
related to threading, such as customized locking schemes or distributed work-stealing.
At least for my purpose, complete threading systems for user-level threading were not needed,
but rather I needed a more flexible implementation to proceed my research.

Implementing user-level threading is not as complicated as many people expect.
To help those people understand the implementation methods easily
and modify the features for individual motivations,
I started to build a new library ComposableThreads.


## Abbreviations

- cmpth = ComposableThreads
- itf = interface
- fdn = fundamental
- sct = shared-memory ComposableThreads
- mth = MassiveThreads
- ult = user-level threads
- klt = kernel-level threads
- ctmth = ComposableThreads (on) MassiveThreads
- exec = execution
- def = default
- buf = buffer

