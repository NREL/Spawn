#ifndef __JMODELICA_H
#define __JMODELICA_H

#include "graal_isolate.h"

#if defined(__cplusplus)
extern "C" {
#endif

void jmodelica_compile(graal_isolatethread_t*, char*);

int jmodelica_create_isolate(graal_create_isolate_params_t* params, graal_isolate_t** isolate, graal_isolatethread_t** thread);

#if defined(__cplusplus)
}
#endif
#endif
