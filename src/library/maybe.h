/* a10 459
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/maybe.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 459 */



/*
  FIXME: This only works with gcc

  MUHAHAHAHAHA!

  Evil macros. Don't combine them in one call.
  (MAYBE_SET_IS_NOTHING (1, MAYBE_IS_NOTHING(2)) is
  bogus for example.
*/

#define MAYBE(type) \
  struct maybe_##type##_t \
  { \
    int is_nothing_p; \
    type value; \
  }

#include <string.h> // for memset

#define MAYBE_INITIALIZE(maybe) \
	({ \
	    typeof (maybe) _maybe = (maybe); \
	    _maybe->is_nothing_p = 1; \
	    memset(&_maybe->value, 0, sizeof(_maybe->value)); \
	})

#define MAYBE_IS_NOTHING(maybe) \
	({ \
	    typeof (maybe) _maybe = (maybe); \
	    !!_maybe->is_nothing_p; \
	})

#define MAYBE_GET_VALUE(maybe) \
	({ \
	    typeof (maybe) _maybe = (maybe); \
	    _maybe->value; \
	})

#define MAYBE_SET_VALUE(maybe, val) \
	({ \
	   typeof (maybe) _maybe = (maybe); \
	   typeof (val) _val = (val); \
	   _maybe->is_nothing_p = !!0; \
	   _maybe->value = _val; \
	})


#define MAYBE_SET_NOTHING(maybe) \
	({ \
	   typeof (maybe) _maybe = (maybe); \
	   _maybe->is_nothing_p = 1; \
	})

#define MAYBE_REF_LVALUE(maybe) \
	({ \
	   typeof (maybe) _maybe = (maybe); \
	   &_maybe->value; \
	})

#define MAYBE_SET_IS_NOTHING(maybe, flag) \
	({ \
	   typeof (flag)  _flag  = (flag); \
	   typeof (maybe) _maybe = (maybe); \
	   _maybe->is_nothing_p = !!_flag; \
	})
