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
    
  if(!freq_p) {
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
  
  return 1000.0*now.time + now.millitm;
}

#elif defined(LINUX) || defined(MACOSX)

#include <libc/sys/time.h>
#include <libc/stdlib.h>

double get_milliseconds()
{
    struct timeval tv;
    double t;
    
    gettimeofday(&tv, NULL);
    t = (1000.0 * tv.tv_sec) + (tv.tv_usec / 1000.0);
    
    return t;
}

double get_unix_milliseconds()
{
  return get_milliseconds();
}

#elif defined(PS2) && defined(EE)

#include <tpolm/lsv700609/cpu.h>

/*
  does not work!
*/
double get_unix_milliseconds()
{
	return get_milliseconds ();
}

// beware, it can wrap around!
double get_milliseconds()
{
	const long c = get_cycle_counter ();
	return (float) c * 1000.f / 300e6f;
}

#else

#error "please implement get_milliseconds and get_unix_milliseconds"

#endif
