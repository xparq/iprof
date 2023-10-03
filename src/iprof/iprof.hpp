// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#pragma once

#ifndef DISABLE_IPROF

#if (defined(_MSC_VER) && (_MSC_VER < 1916)) || defined(EMSCRIPTEN) || defined(CC_TARGET_OS_IPHONE) || defined(__ANDROID__)
#define DISABLE_IPROF_MULTITHREAD
#endif

#ifndef DISABLE_IPROF_OPTIM
#include <stdint.h>
#include <string.h>
#endif

#include <map>
#ifndef DISABLE_IPROF_MULTITHREAD
#include <mutex>
#endif
#include <ostream>
#include <vector>

#include "hitime.hpp"

#ifdef DISABLE_IPROF_MULTITHREAD
// For MSVC: __declspec(thread) doesn't work for things with a constructor
#define iprof_thread_local
#else
#define iprof_thread_local thread_local
#endif

namespace iProf
{
/// A faster (for storing within a vector) but limited vector<const char *>
#ifndef DISABLE_IPROF_OPTIM
class Stack
{
public:
   typedef uint16_t size_type;
   typedef const char* value_type;

protected:
   static const int MAX_DEPTH = 15;
   // Should make sizeof(RawEntry) == 64 (15*4+2+2) for 32bit systems
   // and compilers will align it to 128 for 64 bit systems.
   value_type array[MAX_DEPTH];
   size_type tail = 0;
   size_type osize = 0;  ///< size with overflow

public:
   size_type size() const
   {
      return osize;
   }

   size_type capacity() const
   {
      return tail;
   }

   void push_back(value_type item)
   {
      ++osize;
      if (tail >= MAX_DEPTH)
         return;
      array[tail++] = item;
      return;
   }

   void pop_back()
   {
      if (--osize < MAX_DEPTH)
         --tail;
   }

   const value_type* begin() const { return array; }
   const value_type* end() const { return array + tail; }
   const value_type& back() const { return *(array + tail - 1); }

   bool operator==(const Stack& a) const
   {
      return size() == a.size() && 0 == memcmp(array, a.array, sizeof(value_type) * tail);
   }

   bool operator<(const Stack& a) const
   {
      if (size() < a.size())
         return true;
      else if (size() > a.size())
         return false;
      // We are just comparing pointer values here.
      // The perfect correctness of this depends on compiler putting all
      // string literals (const char *) under the same pointer, which is
      // not guaranteed under the specification, thus inline functions
      // might be a problem for this, and might be threated as different
      // functions when looked at from different translation units.
      return memcmp(array, a.array, sizeof(value_type) * tail) < 0;
   }
}; // class Stack
#else
typedef std::vector<const char*> Stack;
#endif

struct RawEntry
{
   Stack scopes;
   HighResClock::time_point start;
   HighResClock::time_point end;
};

struct Totals
{
   HighResClock::duration totalTime = HighResClock::duration::zero();
   size_t numVisits = 0;
   Totals& operator+=(const Totals& a)
   {
      totalTime += a.totalTime;
      numVisits += a.numVisits;
      return *this;
   }
   Totals& operator-=(const Totals& a)
   {
      totalTime -= a.totalTime;
      numVisits -= a.numVisits;
      return *this;
   }
};

extern iprof_thread_local Stack scopes;
extern iprof_thread_local std::vector<RawEntry> entries;
typedef std::map<Stack, Totals> Stats;   // we lack hashes for unordered
extern iprof_thread_local Stats stats;

#ifndef DISABLE_IPROF_MULTITHREAD
extern std::mutex allThreadStatLock;
extern Stats allThreadStats;
#endif

void aggregateEntries();
void addThisThreadEntriesToAllThreadStats();

inline void Begin(const char* tag)
{
   scopes.push_back(tag);
   auto now = HighResClock::now();
   entries.emplace_back(RawEntry{scopes, now, now - HighResClock::duration(1)});
}
inline void End()
{
   auto s = scopes.size();
   auto rei = entries.rbegin();
   while (rei->scopes.size() != s)
      ++rei;
   rei->end = HighResClock::now();
   scopes.pop_back();
}

struct ScopedMeasure
{
   ScopedMeasure(const char* tag) { Begin(tag); }
   ~ScopedMeasure() { End(); }
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

#define IPROF(n) iProf::ScopedMeasure iProf__##__COUNTER__(n)
#define IPROF_FUNC iProf::ScopedMeasure iProf__##__COUNTER__(__FUNCTION_NAME__)

#else

# define IPROF(n)
# define IPROF_FUNC

#endif
