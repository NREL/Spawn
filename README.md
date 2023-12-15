# Spawn 
Spawn is a software package for performing co-simulations involving EnergyPlus and Modelica.
This package bundles the following items in one self contained package.

1. A method for connecting EnergyPlus models to Modelica
2. A Modelica compiler toolchain for compiling and running Modelica models
3. Modelica libraries and content, including the Modelica Buildings Library (MBL) 
and the Modelica Standard Library.

The Spawn installation package is fully self contained, and there are no external third party dependencies.
Together the capabilities in this package, provide a single integrated environment for performing hybrid Modelica 
and EnergyPlus simulations. The primary entry point is the `spawn` command line interface.

## Installation
Binary packages are published on GitHub, https://github.com/NREL/Spawn/releases.
Extract the package to a location of your choosing and optionally put the `bin/spawn` executable in your system path.

Additional computer platforms, including versions of Mac OS and Windows will be supported in future releases.

## Example Usage
Detailed help is built into the command line program `spawn --help`.

* Compile a Modelica model. Models contained with the Modelica Buildings Library and
the Modelica Standard Library are included and available to the compiler by default.

```shell
spawn modelica create-exe Buildings.Examples.Tutorial.Boiler.System1

```
* Compile and run a Modelica model, which internally leverages EnergyPlus.

```shell
spawn modelica simulate Buildings.ThermalZones.EnergyPlus.Validation.ThermalZone.OneZoneOneYear

```

## Compiling Spawn from source
* Install EnergyPlus dependencies according to https://github.com/NREL/EnergyPlus/wiki/BuildingEnergyPlus
* Ensure that your system has been setup the same as it would be for compiling EnergyPlus, but with a few additions,
* Install clang development libraries. One Linux this would be...

```shell
apt install libllvm10 llvm-10-dev clang-10 libclang-10-dev liblld-10-dev liblld-10-dev gfortran
```

```shell
pip install conan
```

* If neccessary add variables to locate llvm and clang. (not required for apt-get installed linux packages)

```shell
export LLVM_DIR=/path/to/llvm/
export Clang_DIR=/path/to/clang/
```

* Then follow the normal cmake build process.

```shell
git clone --recurse-submodules https://github.com/NREL/spawn.git
cd spawn
mkdir build
cd build
cmake ../
make -j
```

