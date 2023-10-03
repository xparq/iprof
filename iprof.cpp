// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#include "iprof.hpp"

namespace iProf
{
iprof_thread_local Stats stats;
iprof_thread_local std::vector<RawEntry> entries;
iprof_thread_local TagList currentScopePath;

#ifndef IPROF_DISABLE_MULTITHREAD
std::mutex allThreadStatLock;
Stats allThreadStats;
#endif

void aggregateEntries()
{
   std::vector<RawEntry> unfinished;
   if (entries.size() > 4)
      unfinished.reserve(entries.size() >> 1);
   for (auto ei : entries)
   {
      if (ei.tStart > ei.tStop)
      {
         unfinished.emplace_back(ei);
         continue;
      }
      Totals& s = stats[ei.scopePath];
      ++s.nVisits;
      s.tTotal += ei.tStop - ei.tStart;
   }
   std::swap(entries, unfinished);
}

#ifndef IPROF_DISABLE_MULTITHREAD
void addThisThreadEntriesToAllThreadStats()
{
   std::lock_guard<std::mutex> bouncer(allThreadStatLock);
   iprof_thread_local static Stats lastStats;
   for (auto& s : lastStats)
      allThreadStats[s.first] -= s.second;
   for (auto& s : stats)
      allThreadStats[s.first] += s.second;
   lastStats = stats;
}
#endif
} // namespace iProf

std::ostream& operator<<(std::ostream& os, const iProf::Stats& stats)
{
   for (auto [path, data] : stats)
   {
      for (auto& tag : path)
         os << tag << (&tag != &path.back() ? "/" : "");
      if (path.capacity() < path.size())
         os << "/...(" << path.size() - path.capacity() << ")";
      os << ": " << MICRO_SECS(data.tTotal) / float(data.nVisits)
         << " μs (" << MICRO_SECS(data.tTotal)
         << " μs / " << data.nVisits << ")\n";
   }
   return os;
}
