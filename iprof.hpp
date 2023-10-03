// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#pragma once

#ifndef IPROF_DISABLE

#if (defined(_MSC_VER) && (_MSC_VER < 1916)) || defined(EMSCRIPTEN) || defined(CC_TARGET_OS_IPHONE) || defined(__ANDROID__)
# define IPROF_DISABLE_MULTITHREAD
#endif

#ifndef IPROF_DISABLE_OPTIM
# include "TinyPODVector.hpp" // Fast, but very limited POD container
#endif
#include <vector> // std::vector is needed anyway for other things
#include <map>
#ifndef IPROF_DISABLE_MULTITHREAD
# include <mutex>
#endif
#include <ostream>
#include "hitime.hpp"

#ifdef IPROF_DISABLE_MULTITHREAD
// For MSVC: __declspec(thread) doesn't work for things with a constructor
# define iprof_thread_local
#else
# define iprof_thread_local thread_local
#endif

namespace iProf
{
#ifndef IPROF_DISABLE_OPTIM
	typedef TinyPODVector<const char*, 15> TagList;
		// With 15, sizeof == 64 (15*4 + 2 + 2) for 32-bit systems,
		// and should be (aligned to) 128 for 64-bit systems.
#else
	typedef std::vector<const char*> TagList;
#endif

struct RawEntry
{
	TagList scopePath;
	HighResClock::time_point tStart;
	HighResClock::time_point tStop;
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
extern iprof_thread_local std::vector<RawEntry> entries;
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
	auto now = HighResClock::now();
	entries.emplace_back(RawEntry{currentScopePath, now, now - HighResClock::duration(1)}); // start with an invalid interval
}
inline void Stop()
{
	auto depth = currentScopePath.size();
	auto rei = entries.rbegin();
	while (rei->scopePath.size() != depth)
		++rei;
	rei->tStop = HighResClock::now();
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

#define _CONCAT_2_(a, b) a##b
#define _CONCAT_(a, b) _CONCAT_2_(a, b)

#define IPROF(n) iProf::ScopedMeasure _CONCAT_(iProf__,__COUNTER__)(n)
#define IPROF_FUNC iProf::ScopedMeasure _CONCAT_(iProf__,__COUNTER__)(__FUNCTION_NAME__)

#else // IPROF_DISABLE
# define IPROF(n)
# define IPROF_FUNC
#endif // IPROF_DISABLE
