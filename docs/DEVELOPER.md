# Spawn Developer Notes

## Building

This is a cross-platform C++ project that uses CMake. We also use Conan, a C++ package
manager, to manage dependencies. However, some dependencies inherited from EnergyPlus are
not provided by Conan.

A good starting point is to first build standard EnergyPlus by following the instructions
on the EnergyPlus [wiki](https://github.com/NREL/EnergyPlus/wiki/Building-EnergyPlus).
After successfully building EnergyPlus, return here and follow these steps:

1. Install [Conan](https://docs.conan.io/2/reference/commands/install.html).
2. From the root of the project source tree, invoke Conan to download and configure Spawn's
   dependencies. CMake finder files will be generated into the `--output-folder` to help
   CMake locate the dependencies:

   ```bash
   conan install . --output-folder=./build --build=missing \
     -s compiler.cppstd=20 -s build_type=Release
   ```

3. Configure the build using CMake. Conan creates a `CMakeUserPresets.json` file in the
   root of the project, so the only required argument to `cmake` is the name of the preset.
   You can alter preset options using tools like `ccmake` or the CMake GUI:

   ```bash
   cmake --preset conan-release
   ```

4. Build the project:

   ```bash
   cmake --build -j --preset conan-release
   ```

   - You may replace the `Release` build type with `Debug` if debug symbols are required.
   - The file `docs/Dockerfile` defines a container for compiling Spawn or for setting up
     your development environment on Linux.

## Software Design

### Big Picture

Spawn creates a coordinated link between Modelica and EnergyPlus using coroutines, which
are implemented with C++ threads. One thread, the main control thread, is typically a
Modelica process. The other thread contains a running instance of the EnergyPlus
time-marching simulation loop.

The two threads never run simultaneously. Instead, execution passes back and forth, with
one thread waiting while the other runs.

The control thread uses the Spawn API to trigger the EnergyPlus simulation and to read and
write exchanged variables. When the `advance` function is called, the EnergyPlus thread is
signaled to resume, perform a time step, then return control.

State variables, wrapped in a `Variable` class, shuttle data between the threads. Because
the coroutine threads do not execute concurrently, it is safe to read and write variables
from either side. From EnergyPlus's perspective, variables are categorized as inputs,
outputs, or parameters (the latter being constant over time).

### Life of a Spawn Simulation

The following steps outline the typical lifecycle of a Spawn simulation:

1. A Spawn-based Modelica model is identified. This is a plain-text file (or files) using
   Modelica syntax.

2. A Modelica compiler converts this into an executable. Dymola and OpenModelica produce
   native executables by default. Optimica produces a shared library (FMU). Most compilers
   support FMU export.

3. The simulation of the compiled model begins.

4. During initialization, Modelica invokes the Spawn CLI to prepare the EnergyPlus model.
   The IDF file is modified to remove HVAC and control inputs. This produces the
   EnergyPlus FMU, which is placed in the simulation output directory.

5. Still in the initialization phase, the EnergyPlus FMU is loaded and initialized. A
   mostly standard EnergyPlus startup sequence runs, including warmup and sizing (if
   autosizing is used). When ready, EnergyPlus enters the main run period and waits.

6. The Modelica runtime advances the simulation. For each time step, the Spawn API triggers
   EnergyPlus to run a step via the coroutine. Once complete, data is exchanged and the
   simulation continues.

7. At the Modelica stop time, EnergyPlus exits the main loop and performs finalization
   steps. Normal output files are generated, although HVAC outputs are missing since no
   EnergyPlus-side HVAC was simulated.

### Notes

- There may be two FMUs: the model FMU (from the Modelica compiler) and the EnergyPlus
  FMU (generated during simulation).
  
- A working installation of "Spawn" (CLI and `epfmi` library) must be available at
  *simulation* time.
  
- During Modelica *compilation*, only the Modelica models from the Buildings Library are
  required.

## Repository Structure and Key Files

- `coroutine/` — Core link between Modelica and EnergyPlus. Start here to understand how
  Spawn works.

  - `spawn.hpp|cpp` — Defines the main `Spawn` class and its API (`Start`, `Stop`,
    `SetTime`, `GetValue`, `SetValue`, etc.).

  - `variables.hpp|cpp` — Defines the base `Variable` class and derived types for data
    exchange.

- `epfmi/` — Wraps `Spawn` in a C-based FMI-conformant interface. This shared library is
  one of the main outputs. FMI is used for broad compatibility.

- `cli/` — Defines the Spawn command-line interface used during Modelica initialization to
  generate the EnergyPlus FMU.

- `fmu/` — Contains the `FMU` API (`fmu/fmu.hpp`) used for loading and interacting with
  compiled FMUs (mainly for testing).

- `util/` — Miscellaneous utilities supporting the rest of the code.

- `test/` — Integration tests covering end-to-end functionality.

- `energyplus/` — Git subtree of the EnergyPlus repo. Tracks the current EnergyPlus
  release tag.

  - `energyplus/src/EnergyPlus/HeatBalanceAirManager.cc` — `CalcHeatBalanceAir` contains
    a key branching point where EnergyPlus defers to Spawn’s external HVAC manager.

- `mbl/` — CMake logic to pull in and compile the Modelica Buildings Library for testing.

- `fmi2/` — Third-party FMI specification files providing required C data types.

## Testing

Spawn uses [Catch2](https://github.com/catchorg/Catch2) for testing, wrapped in CTest.
To run all tests:

```bash
cmake --preset conan-release -DBUILD_TESTING:BOOL=ON
cmake --build -j --preset conan-release
cd build
ctest -R spawn
```

> Without the `-R spawn` filter, CTest may also run all EnergyPlus tests.

## Redistributable Packages

This project includes CMake install commands to generate binary packages. To build them:

```bash
cmake --preset conan-release -DBUILD_PACKAGE:BOOL=ON
cmake --build -j --preset conan-release --target package
```
