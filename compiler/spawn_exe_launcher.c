
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

// The purpose of this program is the following...
// Modelica supports a feature that enables user defined external functions,
// which can be implemented in the C programming language. During the process of
// compiling Modelica source, the Modelia compiler will call out to the C compiler
// to compile user defined external functions into an executable program.
//
// Unfortunately, technical challenges have prevented the Spawn embedded compiler/linker
// from generating executable programs, however it can generate shared libraries. This main
// program, which is compiled into an exe at spawn build time, is the solution. This 
// precompiled executable program utilizes dlopen to call the just in time compiled shared
// library containing the user defined function. 
//
// See compilerchain.cpp:makeModelicalExternalFunction where this executable is copied into an FMU.
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

