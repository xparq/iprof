// Copyright (c) 2015-2019 Paweł Cichocki
// License: https://opensource.org/licenses/MIT

#pragma once

#include <chrono>
#ifdef _MSC_VER
# include <Windows.h>
#endif

namespace iProf
{
#ifndef _MSC_VER

	using HighResClock = std::chrono::high_resolution_clock;

#else

	namespace
	{
		const auto g_Frequency = []() {
			LARGE_INTEGER frequency;
			QueryPerformanceFrequency(&frequency);
			return frequency.QuadPart;
		}();
	}

	struct HighResClock
	{
		typedef long long                               rep;
		typedef std::nano                               period;
		typedef std::chrono::duration<rep, period>      duration;
		typedef std::chrono::time_point<HighResClock>   time_point;

		static const bool is_steady = true;

		static inline time_point now()
		{
			LARGE_INTEGER count;
			QueryPerformanceCounter(&count);
			return time_point(duration(count.QuadPart * static_cast<rep>(period::den) / g_Frequency));
		}
	};

#endif // _MSC_VER

	inline auto MICROSEC(const HighResClock::duration& x) { return std::chrono::duration_cast<std::chrono::microseconds>(x).count(); }
	inline auto MILLISEC(const HighResClock::duration& x) { return std::chrono::duration_cast<std::chrono::milliseconds>(x).count(); }
	inline auto      SEC(const HighResClock::duration& x) { return std::chrono::duration_cast<std::chrono::seconds>(x).count(); }

} // namespace iProf
