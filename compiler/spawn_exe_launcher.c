
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
  char so_filename[1024];
  so_filename[0] = 0;
  // copy my file name from the OS into the so_filename
  strcpy(so_filename, argv[0]);

  // we're expecting the exact same location, just with .so appended
  strcat(so_filename, ".so");

  void *so = dlopen(so_filename, RTLD_NOW);

  if (!so) {
    fputs("Unable to open expected file", stderr);
    fputs(so_filename, stderr);
    fputs(dlerror(), stderr);
    return EXIT_FAILURE;
  }

  void *so_main = dlsym(so, "main");

  if (!so_main) {
    fputs("Unable to find 'main' symbol in file", stderr);
    fputs(so_filename, stderr);
    fputs(dlerror(), stderr);
  }

  int (*main_func)(int, char **) = so_main;
  return main_func(argc, argv);
}

