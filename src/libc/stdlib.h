/* a10 65
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/libc/stdlib.h') with a license
 * agreement. ('LICENSE' file) 
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 65 */



#ifndef KNOS_LIBC_STDLIB_H
#define KNOS_LIBC_STDLIB_H

#include <stdlib.h>

#if 0 //ARCH == WIN32

// doesnt' work... can't find _aligned functions...

static inline
void* a_calloc(size_t nmemb, size_t size) {
    void *z = _aligned_malloc(nmemb*size, 8);
    memset(z, 0, nmemb*size);

    return z;
}

static inline
void* a_malloc(size_t size) {
    void *z = _aligned_malloc(size, 8);

    return z;
}

static inline
void* a_realloc(void* ptr, size_t new_size) {
    void *z = _aligned_realloc(ptr, new_size, 8);

    return z;
}

static inline
void a_free(void* ptr) {
    _aligned_free(ptr);
}

#elif defined(LINUX)

#include <malloc.h>
#include <string.h> // for memset

static inline
void* a_calloc(size_t nmemb, size_t size) {
    void* z = memalign(8, nmemb*size);
    memset(z, 0, nmemb*size);

    return z;
}

static inline
void* a_malloc(size_t size) {
    void* z = memalign(8, size);

    return z;
}

static inline
void* a_malloc2(size_t size, size_t alignment) {
	return memalign (alignment, size);
}

static inline
void* a_realloc(void* ptr, size_t new_size) {
    void* z = realloc(ptr, new_size);

    return z;
}

static inline
void a_free(void* ptr) {
    free(ptr);
}

#elif defined(PS2)

#include <malloc.h>
#include <string.h> // for memset

static inline
void* a_calloc(size_t nmemb, size_t size) {
    void* z = memalign(128, nmemb*size);
    memset(z, 0, nmemb*size);

    return z;
}

static inline
void* a_malloc(size_t size) {
    void* z = memalign(128, size);

    return z;
}

static inline
void* a_malloc2(size_t size, size_t alignment) {
	return memalign (alignment, size);
}

#include <libc/stdio.h>

static inline
void* a_realloc(void* ptr, size_t new_size) {
	printf ("ERROR!\n, cannot realloc!");
	// does not work
    void* z = realloc(ptr, new_size);

    return z;
}

static inline
void a_free(void* ptr) {
    free(ptr);
}

#else

#define a_calloc calloc
#define a_malloc malloc
#define a_realloc realloc
#define a_free free

static inline
void* a_malloc2(size_t size, size_t alignment) {
	(void) alignment; // unused
	return a_malloc (size);
}

#endif

#endif
