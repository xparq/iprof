// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#include <math.h>
#include <future>
#include <iostream>

#include "hirestime.hpp" // We'll need this also when iprof is disabled (#)
#define IPROF_DISABLE // -> Issue #5
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

	IPROF("Interm."); // For (regression-)testing issue #1

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

//----------------------------------------------------------------------------
int main()
{
	auto startTime = IPROF_NOW;

#ifndef IPROF_DISABLE
	cout << "sizeof(iProf::TagList): " << sizeof(iProf::TagList)
	     << " bytes" << endl;
#endif

	cout << "\nAnd the lucky double is: " << heavyCalc() << endl;

	IPROF_UPDATE;
	cout << "\nThe profiler stats so far:\n"
	        "SCOPE: AVG_TIME (TOTAL_TIME / TIMES_EXECUTED)"
	        "\nAll times in micro seconds\n"
	     << IPROF_STATS << endl;

	cout << "Second lucky double is " << heavyCalc() << endl;

	IPROF_UPDATE;
	cout << "\nThe profiler stats after the second run:\n"
	     << IPROF_STATS << endl;

#ifndef IPROF_DISABLE_MULTITHREAD
	cout << "Let's try a multithread environment" << endl;

	IPROF_SYNC_THREAD;

	auto load = []
	{
		heavyCalc();
		IPROF_UPDATE;
		IPROF_SYNC_THREAD;
	};

	auto futureLucky = std::async(load);
	auto futureLucky2 = std::async(load);

	futureLucky.get();
	futureLucky2.get();

	{
		// In case some threads would still be adding their entries to all thread stats:
		// std::lock_guard<std::mutex> bouncer(iProf::allThreadStatLock);
		cout << "\nThe all-threads profiler stats:\n"
		     << IPROF_ALL_THREAD_STATS << endl;
		/*!! Not this; see #9!...
		cout << "\nThread-local stats (main):\n"
		     << IPROF_STATS << endl;
		!!*/
	}
#else
	cout << "iprof multithreading disabled\n" << endl;
#endif

	cout << "The test took " << IPROF_MILLISEC(IPROF_NOW - startTime)
	     << " milliseconds\n" << endl;
	return 0;
}
