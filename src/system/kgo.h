/* a10 337
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/system/kgo.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 337 */



#ifndef KGO_H
  #define KGO_H

/*
  video system
*/

#include <system/event_listener.h>
#include <system/demo.h>
#include <system/kgo_driver.h>

/*
  open a window using the specified driver
*/
int kgo_open(const char* driver, char* title, int width, int height);

/*
  close the window
*/
int kgo_close(void);

/*
  starts the main displaying loop
*/
void kgo_start(void);

/*
  stops the main displaying loop
*/
void kgo_stop(void);

/*
  get the file descriptor associated with the window
*/
int kgo_get_fd(void);

/*
  get the event listener associated with the window
*/
event_listener_t* kgo_get_event_listener();

/*
  update the window display using the provided buffer.
  if event_pending is 1, process the pending events.
*/
void kgo_update(void* buffer, int event_pending);

/*
  the driver may need to alter certain settings of the demo
*/
int kgo_configure_demo(demo_t* demo);

/*
  return the symbol associated with the frame type supported by this driver.
*/
atom_t kgo_get_frame_type();

/*
  allocate one frame worth 
 */
void* kgo_allocate_frame();

/*
  returns the current driver being used.
*/
kgo_driver_t* kgo_get_current_driver ();

/*
  returns an arbitrary driver for this frame type
*/
kgo_driver_t* kgo_get_driver (atom_t a);

#endif
