// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#include <math.h>
#include <future>
#include <iostream>

#include "iprof.hpp"

using namespace std;

// Senseless calculations func 1
double bigWave()
{
   IPROF_FUNC;

   double ret = 0;
   for (int i = 0; i < 10000; ++i)
      ret += sin(i/1000) - ret * 0.9;
   return ret;
}

// Senseless calculations func 2
double hugePower()
{
   IPROF_FUNC;

//!!   IPROF("Outer; see issue #1!");

   double ret = 2;
   {
      IPROF("FirstPowerLoop");
      for (int i = 0; i < 5000; ++i)
      {
         double exp = (i % 10 + 1) / 7.8;
         ret = pow(ret * 1.4, exp);
      }
   }
   {
      IPROF("SecondPowerLoop");
      for (int i = 0; i < 5000; ++i)
      {
         double exp = double(i & 15) * 0.08;
         ret = pow(ret * 1.4, exp);
      }
   }
   {
      IPROF("BigWavePowerLoop");
      for (int i = 0; i < 3; ++i)
         ret -= bigWave();
   }

   return ret;
}

// Senseless calculations func 3
double heavyCalc()
{
   IPROF_FUNC;

   double ret = 0;
   for (int i = 0; i < 1000; ++i)
   {
      ret += bigWave();
      ret -= hugePower();
   }
   return ret;
}


int main()
{
   auto startTime = HighResClock::now();

   cout << "sizeof(iProf::TagList): " << sizeof(iProf::TagList)
        << " bytes" << endl;

   cout << "\nAnd the lucky double is: " << heavyCalc() << endl;

   iProf::aggregateEntries();
   cout << "\nThe profiler stats so far:\n"
           "WHAT: AVG_TIME (TOTAL_TIME / TIMES_EXECUTED)"
           "\nAll times in micro seconds\n"
        << iProf::stats << endl;

   cout << "Second lucky double is " << heavyCalc() << endl;

   iProf::aggregateEntries();
   cout << "\nThe profiler stats after the second run:\n"
        << iProf::stats << endl;

#ifndef IPROF_DISABLE_MULTITHREAD
   cout << "Let's try a multithread environment" << endl;

   iProf::addThisThreadEntriesToAllThreadStats();

   auto load = []
   {
      heavyCalc();
      iProf::aggregateEntries();
      iProf::addThisThreadEntriesToAllThreadStats();
   };

   auto futureLucky = std::async(load);
   auto futureLucky2 = std::async(load);

   futureLucky.get();
   futureLucky2.get();

   {
      // In case some threads would still be adding their entries to all thread stats:
      // std::lock_guard<std::mutex> bouncer(iProf::allThreadStatLock);
      cout << "\nThe profiler all thread stats:\n"
           << iProf::allThreadStats << endl;
   }
#else
   cout << "iprof multithread disabled\n" << endl;
#endif

   cout << "The test took " << MILLI_SECS(HighResClock::now() - startTime)
        << " milliseconds\n" << endl;
   return 0;
}
