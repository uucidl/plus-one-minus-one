/* a10 601
 * Copyright (c) 2001-2012 Nicolas Léveillé <knos.free.fr>
 *
 * You should have received this file ('src/library/thread-helper.c') with a
 *license
 * agreement. ('LICENSE' file)
 *
 * Copying, using, modifying and distributing this file are rights
 * covered under this licensing agreement and are conditioned by its
 * full acceptance and understanding.
 * e 601 */

#include <library/thread-helper.h>

#include <libc/stdlib.h>
#include <libc/string.h>

#include <log4c.h>
LOG_NEW_DEFAULT_CATEGORY(KNOS_LIBRARY_THREAD_HELPER);

int kn_thread_create_with_attributes(pthread_t *new_thread,
                                     pthread_attr_t *attr,
                                     void *(*start)(void *), void *arg)
{
    pthread_attr_t *attributes;
    int retval;

    if (attr == NULL) {
        attributes = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));
        pthread_attr_init(attributes);

        if (pthread_attr_setschedpolicy(attributes, SCHED_OTHER)) {
            ERROR1("Cannot set SCHED_OTHER ? ");
        }
    } else {
        attributes = attr;
    }

    retval = pthread_create(new_thread, attributes, start, arg);

    if (attr == NULL) {
        pthread_attr_destroy(attributes);
        free(attributes);
    }

    return retval;
}

int kn_thread_create(pthread_t *new_thread, void *(*start)(void *), void *arg)
{
    return kn_thread_create_with_attributes(new_thread, NULL, start, arg);
}

int kn_thread_create_minimum_priority_with_attributes(pthread_t *new_thread,
                                                      pthread_attr_t *attr,
                                                      void *(*start)(void *),
                                                      void *arg)
{
    pthread_attr_t *attributes;
    struct sched_param *sched_parameters;
    int retval;

    if (attr == NULL) {
        attributes = (pthread_attr_t *)malloc(sizeof(pthread_attr_t));

        pthread_attr_init(attributes);
    } else {
        attributes = attr;
    }

    if (pthread_attr_setschedpolicy(attributes, SCHED_OTHER)) {
        ERROR1("Cannot set SCHED_OTHER ? ");
    }

    sched_parameters = (struct sched_param *)malloc(sizeof(struct sched_param));

    memset(sched_parameters, 0, sizeof(struct sched_param));
    sched_parameters->sched_priority = sched_get_priority_min(SCHED_OTHER);

    if (pthread_attr_setschedparam(attributes, sched_parameters)) {
        ERROR2("Cannot set scheduling priority of %d",
               sched_parameters->sched_priority);
    }

    retval = pthread_create(new_thread, attributes, start, arg);

    if (attr == NULL) {
        pthread_attr_destroy(attributes);
        free(attributes);
    }
    free(sched_parameters);

    return retval;
}

int kn_thread_create_minimum_priority(pthread_t *new_thread,
                                      void *(*start)(void *), void *arg)
{
    return kn_thread_create_minimum_priority_with_attributes(new_thread, NULL,
                                                             start, arg);
}

int kn_thread_create_background_with_attributes(pthread_t *new_thread,
                                                pthread_attr_t *attributes,
                                                void *(*start)(void *),
                                                void *arg)
{
#ifdef WIN32
    return kn_thread_create(new_thread, start, arg);
#else
    return kn_thread_create_minimum_priority_with_attributes(new_thread, NULL,
                                                             start, arg);
#endif
}

int kn_thread_create_background(pthread_t *new_thread, void *(*start)(void *),
                                void *arg)
{
    return kn_thread_create_background_with_attributes(new_thread, NULL, start,
                                                       arg);
}

#if defined(LINUX)
#include <libc/unistd.h>
#include <libc/errno.h>
#elif defined(WIN32)
#include <windows.h>
#endif

void thread_yield()
{
#ifdef LINUX
    if (sched_yield() < 0) {
        ERROR2("sched_yield: %s", strerror(errno));
    }
#elif defined(MACOSX)
    pthread_yield_np();
#elif defined(WIN32)
    Sleep(0);
#else
#warning Please implement yield!
#endif
}
