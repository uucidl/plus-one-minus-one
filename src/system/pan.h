/* a10 166
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/pan.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 166 */

#ifndef PAN_H
#define PAN_H

#include <system/demo.h>
#include <system/audio_effect.h>

/*
  open a playback device
*/
int pan_open(const char *driver, const char *device, int sample_rate);

/*
  start device
*/
void pan_start();

/*
  stop playing
*/
void pan_stop();

/*
  close the playback device
 */
int pan_close();

/*
   returns the number of samples (1 for each frame)
*/
int pan_get_samples_number();

/*
  the sample rate
*/
int pan_get_sample_rate();

/*
  get the file descriptor for the playback device
*/
long int pan_get_fd();

/*
  wether the device is ready for update
*/
int pan_is_ready();

/*
  update the device with the provided buffer
*/
void pan_update(sample_t *samples);

/*
  return the current number of milliseconds
*/
double pan_gettime();

/*
  cohfigure demo object
*/
int pan_configure_demo(demo_t *demo);

#endif
