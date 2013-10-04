#ifndef TIMER_H
#define TIMER_H

#if defined(__linux__)
#define PLATFORM_LINUX

#elif defined(_WIN32) //also works on 64 bit windows
#define PLATFORM_WINDOWS 1

#elif defined(__APPLE__) and defined(__MACH__)
#define PLATFROM_OSX 1

#endif


#if defined(PLATFROM_LINUX) or defined(PLATFROM_OSX)
#include <sys/time.h>

#elif defined(PLATFROM_WINDOWS)
#include <windows.h>

#endif

class CTimer {
public:
	CTimer();
	float elapsedMilli();
	float sinceLastMilli();
	void reset();

private:
#if defined(PLATFROM_LINUX) or defined(PLATFROM_OSX)
	timeval start;
	timeval last;

#elif defined(PLATFROM_WINDOWS)
	__int64 start;
	__int64 last;

	LARGE_INTEGER freq;
#endif

};

inline CTimer::CTimer() {
#if defined(PLATFROM_WINDOWS)
	QueryPerformanceFrequency(&freq);
#endif

	reset();
}

inline void CTimer::reset() {
#if defined(PLATFROM_LINUX) or defined(PLATFROM_OSX)
	gettimeofday(&start, NULL);
	last = start;

#elif defined(PLATFROM_WINDOWS)
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	start /= freq;
	last = start;

#endif

}

inline float CTimer::elapsedMilli() {
#if defined(PLATFROM_LINUX) or defined(PLATFROM_OSX)
	timeval now;
	gettimeofday(&now, NULL);
	return (float) (now.tv_sec - start.tv_sec) * 1000 + (float) (now.tv_usec - start.tv_usec) / 1000;

#elif defined(PLATFROM_WINDOWS)
	__float64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	return (float)(now - start) / freq;

#endif
}

inline float CTimer::sinceLastMilli() {
#if defined(PLATFROM_LINUX) or defined(PLATFROM_OSX)
	timeval now;
	gettimeofday(&now, NULL);
	last = now;
	return (float) (now.tv_sec - last.tv_sec) * 1000 + (float) (now.tv_usec - last.tv_usec) / 1000;

#elif defined(PLATFROM_WINDOWS)
	__float64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	last = now;
	return (float) (now - last) / freq;

#endif
}

#endif /* TIMER_H */
