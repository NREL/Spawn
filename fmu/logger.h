#ifndef spawn_fmu_logger_h_INCLUDED
#define spawn_fmu_logger_h_INCLUDED

#include <fmi2FunctionTypes.h>

// Do nothing logger
inline void fmuNothingLogger(fmi2ComponentEnvironment, fmi2String, fmi2Status, fmi2String, fmi2String, ...)
{
}

#endif // spawn_fmu_logger_h_INCLUDED
