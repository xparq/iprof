// Copyright (c) 2015-2019 Paweł Cichocki
// Copyright (c) 2023 Szabolcs Szász
// License: https://opensource.org/licenses/MIT

#pragma once

#include <stdint.h>
#include <string.h>

template <typename T, unsigned MAX_SIZE = 15>
class TinyPODVector
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

	//! NOTE: If, for example, T == const char* (or any pointer), we are
	//! just comparing the pointers below!
	//! The usefulness of this depends on the compiler putting all
	//! the equivalent const char* string literals under the same
	//! pointer, which is of course not guaranteed by the specification.
	//! E.g. items added by inline functions might be a problem for this,
	//! as they might be treated as different functions across different
	//! translation units...

	bool operator==(const TinyPODVector& a) const
	{
		return size() == a.size() && 0 == memcmp(items, a.items, sizeof(value_type) * tail);
	}

	bool operator<(const TinyPODVector& a) const
	{
		if (size() < a.size())
			return true;
		else if (size() > a.size())
			return false;
		return memcmp(items, a.items, sizeof(value_type) * tail) < 0;
	}
}; // class TinyPODVector
