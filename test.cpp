// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#include <math.h>
#include <future>
#include <iostream>
#include <mutex> // For multithr. testing without iprof

#include "hirestime.hpp" // For timing without iprof

//#define IPROF_DISABLE // Testing #5
//#define IPROF_DISABLE_MULTITHREAD // Testing #12
//#define IPROF_DISABLE_VECTOR_OPT // Testing #13
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

	IPROF("Interm."); // Testing issue #1

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
int run_with_iprof()
{
	auto startTime = IPROF_NOW;

#ifndef IPROF_DISABLE
	cout << "sizeof(iProf::TagList): " << sizeof(iProf::TagList)
	     << " bytes" << endl;
#endif

	cout << "\nAnd the lucky double is: " << heavyCalc() << endl;

	IPROF_SYNC;

	cout << "\nThe profiler stats so far:\n"
	        "SCOPE: AVG_TIME (TOTAL_TIME / TIMES_EXECUTED)"
	        "\nAll times in micro seconds\n"
	     << IPROF_STATS << endl;

	cout << "Second lucky double is " << heavyCalc() << endl;

	IPROF_SYNC;

	cout << "\nThe profiler stats after the second run:\n"
	     << IPROF_STATS << endl;

#ifndef IPROF_DISABLE_MULTITHREAD
	cout << "Let's try a multithreaded environment" << endl;

	IPROF_SYNC_THREAD;

	auto load = [] {
		cout << heavyCalc() << endl;
		IPROF_SYNC;
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
#endif // IPROF_DISABLE_MULTITHREAD

	cout << "The test took " << IPROF_MILLISEC(IPROF_NOW - startTime)
	     << " milliseconds\n" << endl;
	return 0;
}

//----------------------------------------------------------------------------
//============================================================================
//----------------------------------------------------------------------------
// Senseless calculations func 1
double bigWave_noprofile()
{
	double ret = 0;
	for (int i = 0; i < 10000; ++i)
		ret += sin(i/1000) - ret * 0.9;
	return ret;
}

// Senseless calculations func 2
double hugePower_noprofile()
{
	double ret = 2;
	for (int i = 0; i < 5000; ++i)
	{
		double exp = (i % 10 + 1) / 7.8;
		ret = pow(ret * 1.4, exp);
	}
	for (int i = 0; i < 5000; ++i)
	{
		double exp = double(i & 15) * 0.08;
		ret = pow(ret * 1.4, exp);
	}
	for (int i = 0; i < 3; ++i)
		ret -= bigWave();

	return ret;
}

// Senseless calculations func 3
double heavyCalc_noprofile()
{
	double ret = 0;
	for (int i = 0; i < 1000; ++i)
	{
		ret += bigWave();
		ret -= hugePower();
	}
	return ret;
}


int run_without_iprof()
{
	auto startTime = HiResTime::now();

	cout << "\nAnd the lucky double is: " << heavyCalc_noprofile() << endl;
	cout << "Second lucky double is " << heavyCalc_noprofile() << endl;

#ifndef IPROF_DISABLE_MULTITHREAD
	cout << "Let's try a multithreaded environment" << endl;

	auto load = [] {
		cout << heavyCalc_noprofile() << endl;
//		static std::mutex lock;
//		std::lock_guard<std::mutex> bouncer(lock);
//		std::lock_guard<std::mutex> bouncer(iProf::allThreadStatLock);
	};

	auto futureLucky = std::async(load);
	auto futureLucky2 = std::async(load);

	futureLucky.get();
	futureLucky2.get();

#else
	cout << "iprof multithreading disabled\n" << endl;
#endif

	cout << "The test took " << HiResTime::MILLISEC(HiResTime::now() - startTime)
	     << " milliseconds\n" << endl;
	return 0;
}

//----------------------------------------------------------------------------
int main()
{
	run_with_iprof();
	iProf::clear();
	//run_with_iprof();
	run_without_iprof();
}
