# Spawn 

_Version: ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}_

_Build: ${CMAKE_PROJECT_VERSION_BUILD}_


This is the spawn executable. 
The primary purpose is currently to create an FMU given an EnergyPlus idf file.

## Development Builds

The latest development builds are available at the following locations.

* [Linux](https://spawn.s3.amazonaws.com/latest/Spawn-latest-Linux.tar.gz).
* [Mac](https://spawn.s3.amazonaws.com/latest/Spawn-latest-Darwin.tar.gz).
* [Windows](https://spawn.s3.amazonaws.com/latest/Spawn-latest-win64.zip).

## Compiling from source

* Install EnergyPlus dependencies according to https://github.com/NREL/EnergyPlus/wiki/BuildingEnergyPlus

* Ensure that your system has been setup the same as it would be for compiling EnergyPlus, but with one addition,

```shell
pip install conan
```

* Build and install version 9.0 of llvm and clang from source, https://gist.github.com/lefticus/d2069c0284d12a5882d85c78a890f5b8

* Export environment variables to help locate llvm and clang
```shell
export LLVM_DIR=/home/username/llvm/
export Clang_DIR=/home/username/llvm/
```

* Then follow the normal cmake build process.

```shell
git clone --recurse-submodules https://github.com/NREL/spawn.git 
mkdir build
cd build
cmake ../
make -j
```

## Example Usage

* Create a fmu. The .spawn file defines the resources that will be compiled into an EnergyPlus based FMU. 
The most important items are epw and idf files.


```shell
./spawn --export examples/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn

```

The resulting fmu will be located in examples/RefBldgSmallOfficeNew2004_Chicago/MyBuilding.fmu,
and may be tested with pyfmi or other fmu simulation tools.

* Compile a Modelica model to FMU format. Models contained with the Modelica Buildings Library and
the Modelica Standard Library are included and available to the compiler by default

```shell
./spawn --compile Buildings.Examples.Tutorial.Boiler.System1

```

The output of the above command is expected as the following.

```shell
$ ./spawn -c Buildings.Examples.Tutorial.Boiler.System1
Parse Model
Instantiate Model
Flatten Model
Generate C Code
Compile C Code
Model Compiled
```

Resulting in the FMU directory, which includes a fully linked shared library, 
`Buildings_Examples_Tutorial_Boiler_System1.so` corresponding to the compiled version
of the given Modelica model


```shell
$ find Buildings_Examples_Tutorial_Boiler_System1/
Buildings_Examples_Tutorial_Boiler_System1/
Buildings_Examples_Tutorial_Boiler_System1/sources
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_init_independent.c
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_base.c
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_funcs.c
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_base.h
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1.c
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_equ_init.c
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_init_dependent.c
Buildings_Examples_Tutorial_Boiler_System1/sources/Buildings_Examples_Tutorial_Boiler_System1_equ.c
Buildings_Examples_Tutorial_Boiler_System1/resources
Buildings_Examples_Tutorial_Boiler_System1/binaries
Buildings_Examples_Tutorial_Boiler_System1/binaries/Buildings_Examples_Tutorial_Boiler_System1.so
Buildings_Examples_Tutorial_Boiler_System1/out.log
Buildings_Examples_Tutorial_Boiler_System1/modelDescription.xml
```

* List available options

```shell
$ ./spawn -h
Spawn of EnergyPlus
Usage: ./spawn [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -c,--compile TEXT           Compile Modelica model to FMU format
  -e,--export TEXT=spawn.json Export a standalone EnergyPlus based FMU
  --no-zip Needs: --export    Skip compressing the contents of the fmu into a zip archive
  -v,--version                Print version info and exit
```

## Data Exchange Variables

The variables exposed through the FMU are documented here:
https://lbl-srg.github.io/soep/softwareArchitecture.html#coupling-of-envelope-model,
however check the modelDescription.xml contained in the generated fmu because
some variables may not be implemented yet.

