/* a10 257
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/thread-helper.h') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 257 */

#ifndef KNOS_LIBRARY_THREAD_HELPER_H
#define KNOS_LIBRARY_THREAD_HELPER_H

#include <pthread.h>

int kn_thread_create(pthread_t *new_thread, void *(*start)(void *), void *arg);
int kn_thread_create_with_attributes(pthread_t *new_thread,
                                     pthread_attr_t *attributes,
                                     void *(*start)(void *), void *arg);
int kn_thread_create_background(pthread_t *new_thread, void *(*start)(void *),
                                void *arg);
int kn_thread_create_background_with_attributes(pthread_t *new_thread,
                                                pthread_attr_t *attributes,
                                                void *(*start)(void *),
                                                void *arg);
int kn_thread_create_minimum_priority(pthread_t *new_thread,
                                      void *(*start)(void *), void *arg);
int kn_thread_create_minimum_priority_with_attributes(
    pthread_t *new_thread, pthread_attr_t *attributes, void *(*start)(void *),
    void *arg);
void thread_yield();

#endif
