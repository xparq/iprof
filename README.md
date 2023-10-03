﻿# iprof - a pretty simple and performant C++ profiling library

The iprof library let's you measure the performance of your C++ code in real time with little overhead,
both in developer- and run-time. The name is a derivative of **I**nternal**Prof**iler.

## Usage

In any function you want to measure add the IPROF_FUNC macro:

```C++
#include "iprof/iprof.hpp"

// ...

void suspectedPerformanceCulprit()
{
   IPROF_FUNC;
   // ...
}
```

You can also gather stats for any scope easily:

```C++
/// ...
{
   IPROF("HeavyStuff");
   // ...
}
// ...
```

The statistics are gathered in the ```iProf::stats```.
To make iprof performant (you don't want your profiler to perturb the measurements by being slow)
the stats are not aggregated automatically. In an application that has a well defined main loop
such as a game it is recommended to gather them once every main loop iteration.
The stats can be aggregated using ```iProf::aggregateEntries()```.

The stats can also be easily streamed out to text thanks to the provided << operator

```C++
iProf::aggregateEntries();
std::cout << "The latest internal profiler stats:\n"
          << iProf::stats << std::endl;
```

On modern compilers that support thread_local (for objects with a constructor without crashing)
iprof also handles gathering stats across many threads:

```C++
iProf::aggregateEntries();
iProf::addThisThreadEntriesToAllThreadStats();
std::cout << "The latest internal profiler stats from across all threads:\n"
          << iProf::allThreadStats << std::endl;
```

In case some threads might be still aggregating stats use the ```iProf::allThreadStatLock```
mutex to guard the allThreadStats:

```C++
{
    std::lock_guard<std::mutex> bouncer(iProf::allThreadStatLock);
    std::cout << "The latest internal profiler stats from across all threads:\n"
              << iProf::allThreadStats << std::endl;
}
```

The << operator formats the text in the following manner:

```text
WHAT: AVG_TIME (TOTAL_TIME / TIMES_EXECUTED)
```

All times are given in micro seconds.
Sample output:

```text
heavyCalc: 904026 (904026 / 1)
heavyCalc/bigWave: 148.25 (148250 / 1000)
heavyCalc/hugePower: 755.53 (755530 / 1000)
heavyCalc/hugePower/SecondPowerLoop: 32.733 (32733 / 1000)
heavyCalc/hugePower/BigWavePowerLoop: 445.48 (445480 / 1000)
heavyCalc/hugePower/FirstPowerLoop: 276.753 (276753 / 1000)
heavyCalc/hugePower/BigWavePowerLoop/bigWave: 148.288 (444863 / 3000)
```

For more guidance please see ```test.cpp```.

## Building

Just add hitime.hpp, iprof.hpp, and iprof.cpp to your project.
A sample is provided in test.cpp.

To build the sample on unixish systems:

```bash
clang++ -std=c++1z -O3 test.cpp iprof.cpp -Wall -lpthread -o builds/iprofTest.out
```

You can use g++ instead of clang++ if you so choose.

A MSVS project to build the sample is provided under the ```winBuild``` directory.

### Additional options

You can disable the multithreading functionality of iprof by defining ```IPROF_DISABLE_MULTITHREAD```.
In case your call trees are very deep you might want to disable the constant-length vector optimization by defining
```IPROF_DISABLE_OPTIM```.

## Contributing

Just submit a pull request on gitlab or github

https://gitlab.com/Neurochrom/iprof

https://github.com/Neurochrom/iprof

## License

This library is **licensed under the (permissive) [MIT](https://opensource.org/licenses/MIT) license**.

Copyright (c) 2015-2019 Paweł Cichocki

Enjoy ;)
