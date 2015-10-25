/* a10 910
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/traffic.h') with a license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 910 */

#ifndef KNOS_DEMOS_1_1_TRAFFIC_H
#define KNOS_DEMOS_1_1_TRAFFIC_H

/* configure traffic module to freeze datafiles
   to disk */
void traffic_freeze_p(int predicat);
/* configure traffic module to restore datafiles
   from disk instead of fetching them */
void traffic_restore_p(int predicat, const char *base_url);
void *fetch_image(void *rg);
void start_grabbing_images(int max_images);

#endif
