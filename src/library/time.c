/* a10 554
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/time.c') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 554 */

#include <library/time.h>

#if defined(WIN32)

#include <windows.h>

static LARGE_INTEGER freq;
static int freq_p = 0;

double get_milliseconds()
{
    double t;
    static LARGE_INTEGER now;

    if (!freq_p) {
        QueryPerformanceFrequency(&freq);
        freq_p = 1;
    }
    QueryPerformanceCounter(&now);
    t = 1000.0 * now.QuadPart / freq.QuadPart;

    return t;
}

#include <sys/timeb.h>
#include <time.h>

#define _timeb timeb
#define _ftime ftime

double get_unix_milliseconds()
{
    struct _timeb now;
    _ftime(&now);

    return 1000.0 * now.time + now.millitm;
}

#elif defined(LINUX)

#include <time.h>
#include <sys/time.h>

extern double get_milliseconds()
{
    struct timespec tp;
    double t;

    clock_gettime(CLOCK_MONOTONIC, &tp);
    t = (1000.0 * tp.tv_sec) + (tp.tv_nsec / 1000000.0);

    return t;
}

extern double get_unix_milliseconds()
{
    struct timeval tv;
    double t;

    gettimeofday(&tv, NULL);
    t = (1000.0 * tv.tv_sec) + (tv.tv_usec / 1000.0);

    return t;
}

#elif defined(MACOSX)

#include <mach/mach_time.h>
#include <sys/time.h>
#include <log4c.h>

LOG_NEW_DEFAULT_CATEGORY(CLOCK);

static struct mach_timebase_info timebase_info;

static void clock_init() __attribute__((constructor));

static void clock_init()
{
    mach_timebase_info_data_t info;
    if (mach_timebase_info(&info)) {
        ERROR1("Could not initialize clock");
        return;
    }

    timebase_info = info;
}

extern double get_milliseconds()
{
    double micros =
        mach_absolute_time() * timebase_info.numer / timebase_info.denom / 1000;

    return micros / 1000.0;
}

extern double get_unix_milliseconds()
{
    struct timeval tv;
    double t;

    gettimeofday(&tv, NULL);
    t = (1000.0 * tv.tv_sec) + (tv.tv_usec / 1000.0);

    return t;
}

#else

#error "please implement get_milliseconds and get_unix_milliseconds"

#endif
