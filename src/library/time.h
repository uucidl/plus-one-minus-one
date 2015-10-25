/* a10 212
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/time.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 212 */

#ifndef KNOS_LIBRARY_TIME_H
#define KNOS_LIBRARY_TIME_H
/*
  return current number of milliseconds
  in absolute time.
 */

double get_milliseconds();

/*
  milliseconds since the birth of unix (suitable
  for a timespec
*/
double get_unix_milliseconds();

#endif
