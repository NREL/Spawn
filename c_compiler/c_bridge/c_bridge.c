
/// \seealso c_bridge.h for more information on why c_bridge exists and how it is used

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// make sure we are compiling the header in the correct mode
#define C_BRIDGE_IMPL

#include "c_bridge.h"


///
/// Provide the appropriate set of wrapping functions here. Note that they
/// just call into the stdlib versions of the functions
///


// stdio.h

// there is no way to forward a va_args, so we must dispatch to the version
// that can take a va_list
int snprintf_wrap(char *restrict buffer, size_t bufsz, const char *restrict format, ...)
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

  
