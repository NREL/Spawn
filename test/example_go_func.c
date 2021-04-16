#include <stdio.h>


int func();
void call();

int go()
{
  call();
  return func();
}

