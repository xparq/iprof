// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#pragma once

#ifndef IPROF_DISABLE

#if (defined(_MSC_VER) && (_MSC_VER < 1916)) || defined(EMSCRIPTEN) || defined(CC_TARGET_OS_IPHONE) || defined(__ANDROID__)
# define IPROF_DISABLE_MULTITHREAD
#endif

#include "hitime.hpp"
#ifndef IPROF_DISABLE_VECTOR_OPT
# include "lossyvector.hpp" // vector replacement optimized for use in std::map
#endif
#include <vector> // needed anyway for other things
#include <map>
#ifndef IPROF_DISABLE_MULTITHREAD
# include <mutex>
#endif
#include <ostream>

#ifdef IPROF_DISABLE_MULTITHREAD
// For MSVC: __declspec(thread) doesn't work for things with a constructor
# define iprof_thread_local
#else
# define iprof_thread_local thread_local
#endif

namespace iProf
{
#ifndef IPROF_DISABLE_VECTOR_OPT
	typedef LossyVector<const char*, 15> TagList;
		// With 15, sizeof == 64 (15*4 + 2 + 2) for 32-bit systems,
		// and should be (aligned to) 128 for 64-bit systems.
#else
	typedef std::vector<const char*> TagList;
#endif

struct Measurement
{
	TagList scopePath;
	HighResClock::time_point tStart, tStop;

	Measurement(const TagList& path) :
		scopePath(path),
		tStart(HighResClock::now()),
		tStop(tStart - HighResClock::duration(1)) // invalid interval means running()
	{}
	constexpr bool running() { return tStart > tStop; }
};

struct Totals
{
	HighResClock::duration tTotal = HighResClock::duration::zero();
	size_t nVisits = 0;

	Totals& operator+=(const Totals& a)
	{
		tTotal += a.tTotal;
		nVisits += a.nVisits;
		return *this;
	}
	Totals& operator-=(const Totals& a)
	{
		tTotal -= a.tTotal;
		nVisits -= a.nVisits;
		return *this;
	}
};

typedef std::map<TagList, Totals> Stats;   // we lack hashes for unordered
extern iprof_thread_local Stats stats;
extern iprof_thread_local std::vector<Measurement> measurements;
extern iprof_thread_local TagList currentScopePath;

#ifndef IPROF_DISABLE_MULTITHREAD
extern std::mutex allThreadStatLock;
extern Stats allThreadStats;
#endif

void aggregateEntries();
void addThisThreadEntriesToAllThreadStats();

inline void Start(const char* tag)
{
	currentScopePath.push_back(tag);
	measurements.emplace_back(Measurement{currentScopePath});
//	measurements.emplace_back(currentScopePath); // This seemed consistently slower with CLANG! :-o
}
inline void Stop()
{
	auto depth = currentScopePath.size();
	auto m = measurements.rbegin();
	while (m->scopePath.size() != depth)
		++m;
	m->tStop = HighResClock::now();
	currentScopePath.pop_back();
}

struct ScopedMeasure
{
	ScopedMeasure(const char* tag) { Start(tag); }
	~ScopedMeasure() { Stop(); }
};
} // namespace iProf

std::ostream& operator<<(std::ostream& os, const iProf::Stats& stats);

#ifndef __FUNCTION_NAME__
# ifdef _MSC_VER
#  define __FUNCTION_NAME__ __FUNCTION__
# else
#  define __FUNCTION_NAME__ __func__
# endif
#endif

#define _IPROF_CONCAT_2_(a, b) a##b
#define _IPROF_CONCAT_(a, b) _IPROF_CONCAT_2_(a, b)

#define IPROF(n) iProf::ScopedMeasure _IPROF_CONCAT_(iProf__,__COUNTER__)(n)
#define IPROF_FUNC iProf::ScopedMeasure _IPROF_CONCAT_(iProf__,__COUNTER__)(__FUNCTION_NAME__)

#else // IPROF_DISABLE
# define IPROF(n)
# define IPROF_FUNC
#endif // IPROF_DISABLE
