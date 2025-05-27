# Spawn Developer Notes

## Building

This is a cross-platform C++ project that uses CMake and Conan. 

### Prerequisites

* **CMake**: Version from the 3.x series (e.g., 3.31). The project is incompatible with CMake 4.x due to dependencies.
* **Conan**: Version 2.x package manager for C++ dependencies.
* **EnergyPlus**: Some dependencies are inherited from EnergyPlus and not provided by Conan.

> **Note**: It's recommended to first build standard EnergyPlus by following the instructions on the EnergyPlus [wiki](https://github.com/NREL/EnergyPlus/wiki/Building-EnergyPlus) before proceeding.

### Build Steps

1. **Install Conan 2.x**
   
   Install [Conan](https://docs.conan.io/2/reference/commands/install.html) version 2.x (version 2.13.0 is known to work).

2. **Configure Dependencies**
   
   From the project root, run Conan to download and configure dependencies:

   ```bash
   conan install . --output-folder=./build --build=missing \
     -s compiler.cppstd=20 -s build_type=Release
   ```

3. **Configure CMake Build**
   
   Conan creates a `CMakeUserPresets.json` file in the project root:

   ```bash
   cmake --preset conan-release
   ```
   
   > You can modify preset options using `ccmake` or the CMake GUI.

4. **Build the Project**

   ```bash
   cmake --build -j --preset conan-release
   ```

### Build Options

* For debug symbols, replace `Release` with `Debug` in the commands above
* For Linux environments, see `docs/Dockerfile` for a container definition that can compile Spawn or set up your development environment

## Software Design

### Big Picture

Spawn creates a coordinated link between Modelica and EnergyPlus using coroutines, implemented with C++ threads:

* **Main Control Thread**: Typically a Modelica process
* **EnergyPlus Thread**: Runs the EnergyPlus time-marching simulation loop

These threads operate in a cooperative pattern:
- They never run simultaneously
- Execution passes back and forth with one thread waiting while the other runs
- When the control thread calls the `advance` function, the EnergyPlus thread is signaled to:
  1. Resume execution
  2. Perform a time step
  3. Return control

Data exchange occurs through state variables (wrapped in a `Variable` class):
- Variables safely shuttle data between threads (no concurrent execution)
- From EnergyPlus's perspective, variables are categorized as:
  - Inputs
  - Outputs
  - Parameters (constant over time)

### Life of a Spawn Simulation

A typical Spawn simulation follows these steps:

1. **Model Definition**
   - Spawn-based Modelica model is created (plain-text files using Modelica syntax)

2. **Compilation**
   - A Modelica compiler converts the model to executable form:
     - Dymola/OpenModelica → native executables
     - Optimica → shared library (FMU)
     - Most compilers support FMU export

3. **Simulation Start**
   - Compiled model begins execution

4. **Initialization: EnergyPlus Preparation**
   - Modelica invokes Spawn CLI to prepare the EnergyPlus model
   - IDF file is modified to remove HVAC and control inputs
   - EnergyPlus FMU is produced and placed in the simulation output directory

5. **Initialization: EnergyPlus Startup**
   - EnergyPlus FMU is loaded and initialized
   - Standard EnergyPlus startup runs (warmup, sizing if autosizing is used)
   - EnergyPlus enters the main run period and waits

6. **Simulation Time Steps**
   - Modelica runtime advances the simulation
   - For each time step:
     - Spawn API triggers EnergyPlus to run a step via coroutine
     - Data is exchanged
     - Simulation continues

7. **Finalization**
   - At Modelica stop time, EnergyPlus exits the main loop
   - EnergyPlus performs finalization steps
   - Normal output files are generated (without HVAC outputs since no EnergyPlus-side HVAC was simulated)

### Important Notes

* **FMU Distinction**
  - Two different FMUs may be involved:
    1. Model FMU (from the Modelica compiler)
    2. EnergyPlus FMU (generated during simulation)

* **Runtime Requirements**
  - A working installation of "Spawn" (CLI and `epfmi` library) must be available at *simulation* time
  - During Modelica *compilation*, only the Modelica models from the Buildings Library are required

## Repository Structure and Key Files

- **`coroutine/`** — Core link between Modelica and EnergyPlus. Start here to understand how
  Spawn works.

  - `spawn.hpp|cpp` — Defines the main `Spawn` class and API (`Start`, `Stop`,
    `SetTime`, `GetValue`, `SetValue`, etc.).

  - `variables.hpp|cpp` — Defines the base `Variable` class and derived types for data
    exchange.

- **`epfmi/`** — Wraps `Spawn` in a C-based FMI-conformant interface. This shared library is
  one of the main outputs. FMI is used for broad compatibility.

- **`cli/`** — Defines the Spawn command-line interface used during Modelica initialization to
  generate the EnergyPlus FMU.

- **`fmu/`** — Contains the `FMU` API (`fmu/fmu.hpp`) used for loading and interacting with
  compiled FMUs (mainly for testing).

- **`util/`** — Miscellaneous utilities supporting the rest of the code.

- **`test/`** — Integration tests covering end-to-end functionality.

- **`energyplus/`** — Git subtree of the EnergyPlus repo. Tracks the current EnergyPlus
  release tag.

  - `energyplus/src/EnergyPlus/HeatBalanceAirManager.cc` — `CalcHeatBalanceAir` contains
    a key branching point where EnergyPlus defers to Spawn’s external HVAC manager.

- **`mbl/`** — CMake logic to pull in and compile the Modelica Buildings Library for testing.

- **`fmi2/`** — Third-party FMI specification files providing required C data types.

## Testing

Spawn uses [Catch2](https://github.com/catchorg/Catch2) for testing, wrapped in CTest.

### Running Tests

```bash
# Configure with testing enabled
cmake --preset conan-release -DBUILD_TESTING:BOOL=ON
cmake --build -j --preset conan-release

# Run all Spawn tests
cd build
ctest -R spawn
```

> **Note:** Without the `-R spawn` filter, CTest will also run all EnergyPlus tests.

### Running a Single Test

To run a specific test or set of tests:

```bash
cd build
./test/tests [test name or pattern]
```

## Redistributable Packages

This project includes CMake install commands to generate binary packages.

### Building Packages

```bash
# Configure with packaging enabled
cmake --preset conan-release -DBUILD_PACKAGE:BOOL=ON

# Build packages
cmake --build -j --preset conan-release --target package
```

The packages will be created in the `build` directory.
