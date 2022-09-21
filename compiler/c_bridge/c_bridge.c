#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#define C_BRIDGE_IMPL

#include "c_bridge.h"

// stdio.h
int snprintf_wrap(char *restrict buffer, unsigned long long bufsz, const char *restrict format, ...)
{
  int ret = 0;

  /* Declare a va_list type variable */
  va_list myargs;

  /* Initialise the va_list variable with the ... after fmt */

  va_start(myargs, format);

  /* Forward the '...' to vprintf */
  ret = vsnprintf(buffer, bufsz, format, myargs);

  /* Clean up the va_list */
  va_end(myargs);

  return ret;
}

// string.h
void *memset_wrap(void *dest, int ch, unsigned long long count)
{
  return memset(dest, ch, count);
}

// math.h
double cos_wrap(double input)
{
  return cos(input);
}
