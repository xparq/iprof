// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#include "iprof.hpp"

namespace iProf
{
iprof_thread_local Stats stats;
iprof_thread_local std::vector<Measurement> measurements;
iprof_thread_local TagList currentScopePath;

#ifndef IPROF_DISABLE_MULTITHREAD
std::mutex allThreadStatLock;
Stats allThreadStats;
#endif

void aggregateEntries()
{
	std::vector<Measurement> unfinished;
	if (measurements.size() > 4)
		unfinished.reserve(measurements.size() >> 1);
	for (auto measurement : measurements)
	{
		if (measurement.running())
		{
			unfinished.emplace_back(measurement);
			continue;
		}
		Totals& stat = stats[measurement.scopePath];
		++stat.nVisits;
		stat.tTotal += measurement.tStop - measurement.tStart;
	}
	std::swap(measurements, unfinished);
}

#ifndef IPROF_DISABLE_MULTITHREAD
void addThisThreadEntriesToAllThreadStats()
{
	iprof_thread_local static Stats lastStats;
	std::lock_guard<std::mutex> bouncer(allThreadStatLock);
	for (const auto& [path, stat] : lastStats)
		allThreadStats[path] -= stat;
	for (const auto& [path, stat] : stats)
		allThreadStats[path] += stat;
	lastStats = stats;
}
#endif
} // namespace iProf

std::ostream& operator<<(std::ostream& os, const iProf::Stats& stats)
{
	for (const auto& [path, data] : stats)
	{
		for (auto& tag : path)
			os << tag << (&tag != &path.back() ? "/" : "");
		if (path.capacity() < path.size())
			os << "/...(" << path.size() - path.capacity() << ")";
		os << ": " << iProf::HRTime::MICROSEC(data.tTotal) / float(data.nVisits)
			<< " μs (" << iProf::HRTime::MICROSEC(data.tTotal)
			<< " μs / " << data.nVisits << ")\n";
	}
	return os;
}
