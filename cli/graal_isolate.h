#ifndef __GRAAL_ISOLATE_H
#define __GRAAL_ISOLATE_H

/*
 * Structure representing an isolate. A pointer to such a structure can be
 * passed to an entry point as the execution context.
 */
struct __graal_isolate_t;
typedef struct __graal_isolate_t graal_isolate_t;

/*
 * Structure representing a thread that is attached to an isolate. A pointer to
 * such a structure can be passed to an entry point as the execution context,
 * requiring that the calling thread has been attached to that isolate.
 */
struct __graal_isolatethread_t;
typedef struct __graal_isolatethread_t graal_isolatethread_t;

#ifdef _WIN64
typedef unsigned long long __graal_uword;
#else
typedef unsigned long __graal_uword;
#endif


/* Parameters for the creation of a new isolate. */
enum { __graal_create_isolate_params_version = 1 };
struct __graal_create_isolate_params_t {
    int version;                                /* Version of this struct */

    /* Fields introduced in version 1 */
    __graal_uword  reserved_address_space_size; /* Size of address space to reserve */

    /* Fields introduced in version 2 */
    const char    *auxiliary_image_path;                /* Path to an auxiliary image to load. */
    __graal_uword  auxiliary_image_reserved_space_size; /* Reserved bytes for loading an auxiliary image. */

    /* Fields introduced in version 3 */
    int            _reserved_1;                 /* Internal usage, do not use. */
    char         **_reserved_2;                 /* Internal usage, do not use. */
    int            pkey;                        /* Isolate protection key. */
};
typedef struct __graal_create_isolate_params_t graal_create_isolate_params_t;
#endif
