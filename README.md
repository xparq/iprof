# iprof - a pretty simple and performant C++ profiling library

The iprof library let's you measure the performance of your C++ code in real time, with little overhead.

## Usage

First, include `iprof.hpp`.

Then in any function you want to measure, add the `IPROF_FUNC` macro:

```C++
#include "iprof.hpp"

// ...

void suspectedPerformanceCulprit()
{
   IPROF_FUNC;
   // ...
}
```

(Note: it will measure the chunk between the macro and the end of the function.)

You can also gather stats for any other scope easily:

```C++
// ...
{
   IPROF("HeavyStuff");
   // ...
}
// ...
```

The statistics are gathered in `IPROF_STATS`.
To make iprof performant (you don't want your profiler to perturb the measurements by being slow)
the stats are not aggregated automatically. In an application that has a well-defined main loop,
such as a game, it is recommended to gather them once every main loop iteration.
The stats can be aggregated using `IPROF_SYNC`.

The stats can also be easily streamed out to text, thanks to the provided `<<` operator:

```C++
IPROF_SYNC;
std::cerr << "The latest internal profiler stats:\n"
          << IPROF_STATS << '\n;
```

If enabled (i.e. by default) iprof can also handle gathering stats across multiple threads, like:

```C++
IPROF_SYNC;
IPROF_THREAD_SYNC;
std::cerr << "The latest internal profiler stats from across all threads:\n"
          << IPROF_ALL_THREAD_STATS << '\n';
```

In case some threads might be still aggregating stats, use the ```iProf::allThreadStatLock```
mutex to guard `IPROF_ALL_THREAD_STATS`:

```C++
{
    std::lock_guard<std::mutex> bouncer(iProf::allThreadStatLock);
    std::cout << "The latest internal profiler stats from across all threads:\n"
              << IPROF_ALL_THREAD_STATS << '\n';
}
```

The `<<` operator formats the text in the following manner:

```text
SCOPE: AVG_TIME μs (TOTAL_TIME μs / TIMES_EXECUTED)
```

(Only the microsecond unit is supported currently.)

Sample output:

```text
heavyCalc: 904026 μs (904026 μs / 1)
heavyCalc/bigWave: 148.25 μs (148250 μs / 1000)
heavyCalc/hugePower: 755.53 μs (755530 μs / 1000)
heavyCalc/hugePower/SecondPowerLoop: 32.733 μs (32733 μs / 1000)
heavyCalc/hugePower/BigWavePowerLoop: 445.48 μs (445480 μs / 1000)
heavyCalc/hugePower/FirstPowerLoop: 276.753 μs (276753 μs / 1000)
heavyCalc/hugePower/BigWavePowerLoop/bigWave: 148.288 μs (444863 μs / 3000)
```

For more guidance please see `test.cpp`.

## Building

Just add the headers and `iprof.cpp` to your project.

A usage example is also provided in `test.cpp`.
To build it with CLANG (typically on unixish systems):

```bash
clang++ -O3 -Wall -std=c++17 test.cpp iprof.cpp
```

You can of course use `g++` instead, if you wish.

With MSVC:

```cmd
cl /EHsc /O2 /W4 /std:c++17 test.cpp iprof.cpp
```

(An MSVS project to build the example was also provided by the [original upstream repo](https://gitlab.com/Neurochrom/iprof), under its `winBuild` directory.)

### Options

- By defining `IPROF_DISABLE` (before including iprof.hpp) all of iprof will be omitted form the compilation; the `IPROF_...` macros will be defined empty (still passing through their arguments, though, so compilation without iprof would not introduce difficult-to-find subtle bugs).

- You can disable the multithreading functionality by defining `IPROF_DISABLE_MULTITHREAD`.

- In case your scopes are nested way too deep (more than 15), you might want to disable the constant-length vector optimization with
`IPROF_DISABLE_VECTOR_OPT`. You should also call the police and turn yourself in.

## Contributing

See the upstream repos:

- https://gitlab.com/Neurochrom/iprof (OG Paweł Cichocki)
- https://github.com/seanballais/iprof (CMake-ified version by Sean Francis Ballais)

## License

This library is **licensed under the (permissive) [MIT](https://opensource.org/licenses/MIT) license**.

Copyright (c) 2015-2019 Paweł Cichocki,
Copyright (c) 2023 Szabolcs Szász

Enjoy ;)
