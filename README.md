# Spawn 

_Version: ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}_

_Build: ${CMAKE_PROJECT_VERSION_BUILD}_


This is the spawn executable for doing many great things. 
Most importantly you can create a FMU given an EnergyPlus idf file.

## Development Builds

The latest development builds are available at the following locations.

* [Linux](https://spawn.s3.amazonaws.com/latest/Spawn-latest-Linux.tar.gz).

## Example Usage

Create a fmu

  spawn -c examples/RefBldgSmallOfficeNew2004_Chicago/RefBldgSmallOfficeNew2004_Chicago.spawn

