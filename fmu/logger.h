#ifndef spawn_fmu_logger_h_INCLUDED
#define spawn_fmu_logger_h_INCLUDED

#include <fmi2FunctionTypes.h>
#include <stdarg.h>

extern "C" {
//NOLINTNEXTLINE
inline void fmuStdOutLogger(
    fmi2ComponentEnvironment /*comp*/, fmi2String /*name*/, fmi2Status /*level*/, fmi2String /*type*/, fmi2String format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
  printf("\n");
  fflush(stdout);
}

// NOLINTNEXTLINE
inline void fmuNothingLogger(
    fmi2ComponentEnvironment /*comp*/, fmi2String /*name*/, fmi2Status /*level*/, fmi2String /*type*/, fmi2String /*format*/, ...)
{
}
}

#endif // spawn_fmu_logger_h_INCLUDED
