/* a10 144
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/memory.c') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 144 */




#include  <library/memory.h>

#include <libc/strings.h>
#include <libc/string.h>
#include <libc/stdlib.h>
#include <libc/pthread.h>
#include <library/stack.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_MEMORY);

#include <library/memory_impl.h>

static const unsigned short magic = 0x3D21;
static const unsigned short magic_retired = ~0x3D21;

#ifndef NDEBUG
object_t* object_allocate_debug(object_t* x, size_t size, const char* class_name, const char* source_file, int source_line)
{
  object_t* o = x ? x : memory_allocate (size);

  if (x == NULL) {
      /* mark as not allocated for a short instant */
      o->magic = magic_retired;
  }
  
  if (o->magic != magic) {
      memset(o, 0, size);
      o->magic	    = magic;
      o->class_name = class_name;
      o->source_file = source_file;
      o->source_line = source_line;
      o->size       = size;
  }
   
  if(x == NULL) {
    o->allocator = memory_allocator();
  }

  return o;
}

#endif

object_t* object_allocate(object_t* x, size_t size)
{
  object_t* o = x ? x : memory_allocate(size);

  if (x == NULL) {
      /* mark as not allocated for a short instant */
      o->magic = magic_retired;
  }
  
  if (o->magic != magic) {
      memset(o, 0, size);
      o->magic = magic;
      o->size  = size;
  }
   
  if(x == NULL)
      o->allocator = memory_allocator();

  return o;
}

object_t* object_allocate_copy (object_t* from)
{
    if (from && from->size > 0) {
	object_t* o = object_allocate (NULL, from->size);
	memcpy (o, from, from->size);

	return o;
    } else {
	return NULL;
    }
}

void object_set_release_callback (object_t* o, int (*f)(object_t*)) {
    o->release = f;
}

void object_retire(object_t* x)
{
    if(x->magic != magic) {
#ifndef NDEBUG
      ERROR3("object '%x' of class '%s' hasn't been instantiated.", x, x->class_name);
#else
      ERROR2("object '%x' hasn't been instantiated.", x);
#endif
    } else {
	x->magic = magic_retired;

	if (x->release) {
	    int success = x->release (x);
	    if (!success) {
#ifndef NDEBUG
		ERROR3("object '%x' of class '%s' could not be released.", x, x->class_name);
#else
		ERROR2("object '%x' could not be released.", x);
#endif
	    }
	}

	if (x->allocator) {
	    allocator_t* a = x->allocator;
	    x->allocator = NULL;
	    
	    a->retire(a, x);
	}
    }
}

static
void* libc_allocate(allocator_t* self, size_t n)
{
    return malloc(n);
}

static
void libc_free(allocator_t* self, void* ptr)
{
    free(ptr);
}

allocator_t* allocator_instantiate (allocator_t* x) {
    return allocator_instantiate_super (x);
}

allocator_t* libc_allocator_instantiate(allocator_t* x)
{
    allocator_t* a = OBJECT_INSTANTIATE (allocator, x);
    
    a->allocate = libc_allocate;
    a->retire   = libc_free;

    return a;
}

static
void* alibc_allocate(allocator_t* self, size_t n)
{
    return a_malloc(n);
}

static
void alibc_free(allocator_t* self, void* ptr)
{
    a_free(ptr);
}

allocator_t* alibc_allocator_instantiate(allocator_t* x)
{
    allocator_t* a = OBJECT_INSTANTIATE (allocator, x);
    
    a->allocate = alibc_allocate;
    a->retire     = alibc_free;

    return a;
}

static
void* noop_allocate(allocator_t* self, size_t n)
{
    ERROR1("You are not supposed to call this.");
    return NULL;
}

static
void noop_free(allocator_t* self, void* ptr)
{
    // do nothing
}

allocator_t* noop_allocator_instantiate(allocator_t* x)
{
    allocator_t* a = OBJECT_INSTANTIATE (allocator, x);
    
    a->allocate = noop_allocate;
    a->retire   = noop_free;

    return a;
}

static allocator_t noop_allocator;
static int noop_allocator_p = 0;

allocator_t* noop_get_allocator()
{
    if(!noop_allocator_p) {
	noop_allocator_instantiate (&noop_allocator);
    }

    return &noop_allocator;
}

static allocator_t libc_allocator;
static int libc_allocator_p = 0;

allocator_t* libc_get_allocator()
{
    if(!libc_allocator_p) {
	libc_allocator_instantiate (&libc_allocator);
    }

    return &libc_allocator;
}

static allocator_t alibc_allocator;
static int alibc_allocator_p = 0;

allocator_t* alibc_get_allocator()
{
    if(!alibc_allocator_p) {
	alibc_allocator_instantiate (&alibc_allocator);
    }

    return &alibc_allocator;
}

/*
  return (or create if necessary) the per-thread allocator stack
*/

#define DISABLE_PTHREAD ((!defined(KOS) && !defined(PS2) && !defined(PSP)))

#if DISABLE_PTHREAD
static pthread_mutex_t stack_key_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t stack_key;
#else
  static kn_stack_t* stack = NULL;
#endif
static int stack_key_init_p = 1;

#if DISABLE_PTHREAD
static pthread_mutex_t allocator_override_key_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_key_t allocator_override_key;
#else
  static allocator_t* allocator = NULL;
#endif
static int allocator_override_key_init_p = 1;

static
void init_allocator_override_key()
{
    if(allocator_override_key_init_p) {
#if DISABLE_PTHREAD
	if(pthread_mutex_lock(&allocator_override_key_mutex)) {
	    ERROR1("error, lock stack key mutex");
	}
#endif
	if(allocator_override_key_init_p) {
#if DISABLE_PTHREAD
	    if(pthread_key_create(&allocator_override_key, free)) {
		ERROR1("error, stack key create");
	    } else
#endif
		allocator_override_key_init_p = 0;
	}
#if DISABLE_PTHREAD
	if(pthread_mutex_unlock(&allocator_override_key_mutex)) {
	    ERROR1("error, unlock stack key mutex");
	}
#endif
    }
}

static
allocator_t* get_allocator_override()
{
    init_allocator_override_key();
#if DISABLE_PTHREAD
    return pthread_getspecific(allocator_override_key);
#else
    return allocator;
#endif
}

static
void set_allocator_override(allocator_t* a)
{
    init_allocator_override_key();
#if DISABLE_PTHREAD
    pthread_setspecific(allocator_override_key, a);
#else
    allocator = a;
#endif
}

static
kn_stack_t* get_allocator_stack()
{
#if DISABLE_PTHREAD
    if(stack_key_init_p) {
	if(pthread_mutex_lock(&stack_key_mutex)) {
	    ERROR1("error, lock stack key mutex");
	}
	if(stack_key_init_p) {
	    if(pthread_key_create(&stack_key, free)) {
	      ERROR1("error, stack key create");
	    }
	}
	if(pthread_mutex_unlock(&stack_key_mutex)) {
	    ERROR1("error, unlock stack key mutex");
	}
    }

    if(pthread_getspecific(stack_key) == NULL) {
#else
    if(stack == NULL) {
#endif
      kn_stack_t* s;
      /* override allocator to avoid an infinite loop in kn_stack_instantiate */
      set_allocator_override(libc_get_allocator());
      s = kn_stack_instantiate_toplevel (NULL);
      s->push(s, libc_get_allocator());

#if DISABLE_PTHREAD
      pthread_setspecific(stack_key, s);
#else
      stack = s;
#endif
      /* disable override */
      set_allocator_override(NULL);
      stack_key_init_p = 0;
    }

#if DISABLE_PTHREAD
    return pthread_getspecific(stack_key);
#else
    return stack;
#endif
}

void  push_allocator(allocator_t* a)
{
    kn_stack_t* s = get_allocator_stack();

    s->push(s, a);
}

allocator_t* get_top_allocator()
{
    allocator_t* a = get_allocator_override();
    if(a == NULL) {
	kn_stack_t* s = get_allocator_stack();
	
	a = s->top(s);
    }

    return a;
}

allocator_t* pop_allocator()
{
    kn_stack_t* s = get_allocator_stack();
    allocator_t* a = get_allocator_override();
    allocator_t* b;

    if(s->count(s) == 1) {
	ERROR1("tried to pop top allocator.");
	b = s->top(s);
    } else {
	b = s->pop(s);
    }

    if(a != NULL) 
	return a;
    else
	return b;
}

