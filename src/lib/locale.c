/* a10 331
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/lib/locale.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 331 */




#include "locale.h"
#include <libc/ctype.h>


#ifdef WIN32

#include <windows.h>

static char code[5];

const char* get_locale()
{
  char buffer[4];
  int n = GetLocaleInfo(LOCALE_USER_DEFAULT, 
			LOCALE_SABBREVLANGNAME,
			&buffer, 4);
  
  if(n) {
    code[0] = tolower(buffer[0]);
    code[1] = tolower(buffer[1]);
    code[2] = '\0';
  } else {
    code[0] = '\0';
  }
  
  return code;
}

#else

#include <locale.h>

static char code[3];

const char* get_locale()
{
    const char* pof = setlocale(LC_ALL, "");
    if(pof) {
      code[0] = tolower(pof[0]);
      code[1] = tolower(pof[1]);
      code[2] = '\0';
    } else {
      code[0] = '\0';
    }

    return code;
}

#endif
