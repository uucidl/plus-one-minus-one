/* a10 133
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/locale.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 133 */



#ifndef KNOS_DEMOS_LIB_LOCALE_H
#define KNOS_DEMOS_LIB_LOCALE_H

/* returns a 2 letter code of the current locale 
   not thread-safe
*/
const char* get_locale();

#endif
