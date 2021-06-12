#ifndef __OPTIMICA_H
#define __OPTIMICA_H

#include "graal_isolate.h"

#if defined(__cplusplus)
extern "C" {
#endif

void optimica_compile(graal_isolatethread_t*, char*);

int optimica_create_isolate(graal_create_isolate_params_t* params, graal_isolate_t** isolate, graal_isolatethread_t** thread);

#if defined(__cplusplus)
}
#endif
#endif
