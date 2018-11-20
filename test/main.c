#include "test-config.h"
#include "FMUEnergyPlusStructure.h"
#include <stdio.h>


void* FMUZoneInit(const char* idfName, const char* weaName, const char* iddName, const char* epLibName, const char* zoneName);
void FMUZoneInstantiate(void* object, double t0, double* AFlo, double* V, double* mSenFac);

int main() {
  //void* FMUZoneInit(const char* idfName, const char* weaName, const char* iddName, const char* epLibName, const char* zoneName)
  printf("main 1\n");
  FMUZone* zone = FMUZoneInit(input,weather,idd,fmulib,"Core_ZN");
  printf("main 2\n");
  double t0 = 0.0;
  double AFlo = 0.0;
  double V = 0.0;
  double mSenFac = 0.0;
  printf("main 3\n");
  FMUZoneInstantiate(zone, t0, &AFlo, &V, &mSenFac);
  printf("main 4\n");
  return 0;
}
