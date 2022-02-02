#include "../fmi2/fmi2FunctionTypes.h"

extern "C" {

#ifdef _MSC_VER
__declspec(dllexport)
#endif
    const char *fmi2GetVersion()
{
  return "TEST_VERSION";
}
}
