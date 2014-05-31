#ifndef TIMER_H
#define TIMER_H

#if (defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))

#define UNIX_LIKE
#include <sys/time.h>

#elif defined(_WIN32)
#include <windows.h>

#else
#error Platform not supported

#endif

class CTimer {
public:
	CTimer();
	float elapsedMilli();
	float sinceLastMilli();
	void reset();

private:
#ifdef UNIX_LIKE
	timeval start;
	timeval last;

#else
	__int64 start;
	__int64 last;

	LARGE_INTEGER freq;
#endif

};

inline CTimer::CTimer() {
#ifndef UNIX_LIKE //windows here
	QueryPerformanceFrequency(&freq);
#endif

	reset();
}

inline void CTimer::reset() {
#ifdef UNIX_LIKE
	gettimeofday(&start, NULL);
	last = start;

#else
	QueryPerformanceCounter((LARGE_INTEGER*)&start);
	start /= freq;
	last = start;

#endif

}

inline float CTimer::elapsedMilli() {
#ifdef UNIX_LIKE
	timeval now;
	gettimeofday(&now, NULL);
	return (float) (now.tv_sec - start.tv_sec) * 1000 + (float) (now.tv_usec - start.tv_usec) / 1000;

#else
	__float64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	return (float)(now - start) / freq;

#endif
}

inline float CTimer::sinceLastMilli() {
#ifdef UNIX_LIKE
	timeval now;
	gettimeofday(&now, NULL);
	last = now;
	return (float) (now.tv_sec - last.tv_sec) * 1000 + (float) (now.tv_usec - last.tv_usec) / 1000;

#else
	__float64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	last = now;
	return (float) (now - last) / freq;

#endif
}

#endif /* TIMER_H */
