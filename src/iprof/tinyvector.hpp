// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#pragma once

#include <cstddef>
#include <cstring>

template <typename T, unsigned MAX_SIZE = 15>
   // With 15, sizeof == 64 (15*4 + 2 + 2) for 32-bit systems,
   // and should be (aligned to) 128 for 64-bit systems.
class TinyVector
{
public:
   typedef uint16_t size_type;
   typedef T value_type;

protected:
   value_type items[MAX_SIZE];
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
      if (tail >= MAX_SIZE)
         return;
      items[tail++] = item;
      return;
   }

   void pop_back()
   {
      if (--osize < MAX_SIZE)
         --tail;
   }

   const value_type* begin() const { return items; }
   const value_type* end() const { return items + tail; }
   const value_type& back() const { return *(items + tail - 1); }

   bool operator==(const TinyVector& a) const
   {
      return size() == a.size() && 0 == memcmp(items, a.items, sizeof(value_type) * tail);
   }

   bool operator<(const TinyVector& a) const
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
      return memcmp(items, a.items, sizeof(value_type) * tail) < 0;
   }
}; // class TinyVector
