// EnergyPlus, Copyright (c) 1996-2024, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <cmath>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FaultsManager.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>

namespace EnergyPlus {

namespace HVACDXHeatPumpSystem {
    // Module containing the DXHeatPumpSystem simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Brent Griffith (derived by R.Liesen)
    //       DATE WRITTEN   May 2011
    //                      Feb 2013, Bo Shen, Oak Ridge National Lab
    //                      Add Coil:Heating:DX:VariableSpeed
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the DX Heat Pump System System Component
    // this wraps heat pump air-heating coils in coil-only wrapper with no fans.

    // METHODOLOGY EMPLOYED:

    // REFERENCES:

    // OTHER NOTES:

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataHVACGlobals;
    using namespace ScheduleManager;

    void SimDXHeatPumpSystem(EnergyPlusData &state,
                             std::string_view DXHeatPumpSystemName,            // Name of DXSystem:Airloop object
                             bool const FirstHVACIteration,                    // True when first HVAC iteration
                             int const AirLoopNum,                             // Primary air loop number
                             int &CompIndex,                                   // Index to CoilSystem:Heating:DX object
                             ObjexxFCL::Optional_int_const OAUnitNum,          // If the system is an equipment of OutdoorAirUnit
                             ObjexxFCL::Optional<Real64 const> OAUCoilOutTemp, // the coil inlet temperature of OutdoorAirUnit
                             ObjexxFCL::Optional<Real64> QTotOut               // the total cooling output of unit
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith (derived by R.Liesen)
        //       DATE WRITTEN   May 2011
        //                      Feb 2013, Bo Shen, Oak Ridge National Lab
        //                      Add Coil:Heating:DX:VariableSpeed

        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages DXHeatPumpSystem component simulation.

        // Using/Aliasing
        using DXCoils::SimDXCoil;

        using VariableSpeedCoils::SimVariableSpeedCoils;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string CompName; // Name of CoilSystem:Heating:DX object
        int DXSystemNum;      // Index to CoilSystem:Heating:DX object
        Real64 AirMassFlow;   // DX System air mass flow rate

        auto &DXHeatPumpSystem(state.dataHVACDXHeatPumpSys->DXHeatPumpSystem);

        // Obtains and Allocates DX Cooling System related parameters from input file
        if (state.dataHVACDXHeatPumpSys->GetInputFlag) { // First time subroutine has been entered
            // Get the DXCoolingSystem input
            GetDXHeatPumpSystemInput(state);
            state.dataHVACDXHeatPumpSys->GetInputFlag = false;
        }

        int NumDXHeatPumpSystems = state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems;

        // Find the correct DXSystemNumber
        if (CompIndex == 0) {
            DXSystemNum = Util::FindItemInList(DXHeatPumpSystemName, DXHeatPumpSystem);
            if (DXSystemNum == 0) {
                ShowFatalError(state, format("SimDXHeatPumpSystem: DXUnit not found={}", DXHeatPumpSystemName));
            }
            CompIndex = DXSystemNum;
        } else {
            DXSystemNum = CompIndex;
            if (DXSystemNum > NumDXHeatPumpSystems || DXSystemNum < 1) {
                ShowFatalError(state,
                               format("SimDXHeatPumpSystem:  Invalid CompIndex passed={}, Number of DX Units={}, DX Unit name={}",
                                      DXSystemNum,
                                      NumDXHeatPumpSystems,
                                      DXHeatPumpSystemName));
            }
            if (state.dataHVACDXHeatPumpSys->CheckEquipName(DXSystemNum)) {
                if (DXHeatPumpSystemName != DXHeatPumpSystem(DXSystemNum).Name) {
                    ShowFatalError(state,
                                   format("SimDXHeatPumpSystem: Invalid CompIndex passed={}, DX Unit name={}, stored DX Unit Name for that index={}",
                                          DXSystemNum,
                                          DXHeatPumpSystemName,
                                          DXHeatPumpSystem(DXSystemNum).Name));
                }
                state.dataHVACDXHeatPumpSys->CheckEquipName(DXSystemNum) = false;
            }
        }

        if (present(OAUnitNum)) {
            InitDXHeatPumpSystem(state, DXSystemNum, AirLoopNum, OAUnitNum, OAUCoilOutTemp);
        } else {
            InitDXHeatPumpSystem(state, DXSystemNum, AirLoopNum);
        }

        // Call the series of components that simulate a DX Heating System
        // Control the DX Heating System
        ControlDXHeatingSystem(state, DXSystemNum, FirstHVACIteration);

        // simulate DX Heating System
        CompName = DXHeatPumpSystem(DXSystemNum).HeatPumpCoilName;

        switch (DXHeatPumpSystem(DXSystemNum).HeatPumpCoilType_Num) {
        case CoilDX_HeatingEmpirical: { // COIL:DX:COOLINGBYPASSFACTOREMPIRICAL
            SimDXCoil(state,
                      CompName,
                      CompressorOperation::On,
                      FirstHVACIteration,
                      DXHeatPumpSystem(DXSystemNum).HeatPumpCoilIndex,
                      DXHeatPumpSystem(DXSystemNum).FanOpMode,
                      DXHeatPumpSystem(DXSystemNum).PartLoadFrac);
        } break;
        case Coil_HeatingAirToAirVariableSpeed: { // Coil:Heating:DX:VariableSpeed
            SimVariableSpeedCoils(state,
                                  CompName,
                                  DXHeatPumpSystem(DXSystemNum).HeatPumpCoilIndex,
                                  DXHeatPumpSystem(DXSystemNum).FanOpMode,
                                  CompressorOperation::On,
                                  DXHeatPumpSystem(DXSystemNum).PartLoadFrac,
                                  DXHeatPumpSystem(DXSystemNum).SpeedNum,
                                  DXHeatPumpSystem(DXSystemNum).SpeedRatio,
                                  state.dataHVACDXHeatPumpSys->QZnReq,
                                  state.dataHVACDXHeatPumpSys->QLatReq,
                                  state.dataHVACDXHeatPumpSys->OnOffAirFlowRatio);
        } break;
        default: {
            ShowFatalError(state, format("SimDXCoolingSystem: Invalid DX Heating System/Coil={}", DXHeatPumpSystem(DXSystemNum).HeatPumpCoilType));
        } break;
        }
        // set econo lockout flag
        // set econo lockout flag
        if (AirLoopNum != -1) { // IF the sysem is not an equipment of outdoor air unit

            if ((DXHeatPumpSystem(DXSystemNum).PartLoadFrac > 0.0) &&
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithCompressor) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ReqstEconoLockoutWithCompressor = true;
            } else {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ReqstEconoLockoutWithCompressor = false;
            }
        }

        if (present(QTotOut)) {
            int InletNodeNum = DXHeatPumpSystem(DXSystemNum).DXHeatPumpCoilInletNodeNum;
            int OutletNodeNum = DXHeatPumpSystem(DXSystemNum).DXHeatPumpCoilOutletNodeNum;
            AirMassFlow = state.dataLoopNodes->Node(OutletNodeNum).MassFlowRate;
            QTotOut = AirMassFlow * (state.dataLoopNodes->Node(InletNodeNum).Enthalpy - state.dataLoopNodes->Node(OutletNodeNum).Enthalpy);
        }
    }

    // Get Input Section of the Module
    //******************************************************************************

    void GetDXHeatPumpSystemInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith (derived by R.Liesen)
        //       DATE WRITTEN   May 2011
        //                      Feb 2013, Bo Shen, Oak Ridge National Lab
        //                      Add Coil:Heating:DX:VariableSpeed
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for system and stores it in System data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // REFERENCES:

        // Using/Aliasing
        using BranchNodeConnections::SetUpCompSets;
        using BranchNodeConnections::TestCompSet;
        using DXCoils::GetCoilInletNode;
        using DXCoils::GetCoilOutletNode;
        using DXCoils::SetCoilSystemHeatingDXFlag;
        using VariableSpeedCoils::GetCoilInletNodeVariableSpeed;
        using VariableSpeedCoils::GetCoilOutletNodeVariableSpeed;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;
        int NumNums;
        int IOStat;
        static constexpr std::string_view RoutineName("GetDXHeatPumpSystemInput: "); // include trailing blank space
        bool IsNotOK;                                                                // Flag to verify name
        int DXHeatSysNum;
        std::string CurrentModuleObject; // for ease in getting objects
        Array1D_string Alphas;           // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> Numbers;         // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

        auto &DXHeatPumpSystem(state.dataHVACDXHeatPumpSys->DXHeatPumpSystem);

        CurrentModuleObject = "CoilSystem:Heating:DX";
        // Update Num in state and make local convenience copy
        int NumDXHeatPumpSystems = state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        DXHeatPumpSystem.allocate(NumDXHeatPumpSystems);
        state.dataHVACDXHeatPumpSys->CheckEquipName.dimension(NumDXHeatPumpSystems, true);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, "CoilSystem:Heating:DX", state.dataHVACDXHeatPumpSys->TotalArgs, NumAlphas, NumNums);

        Alphas.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNums);
        Numbers.dimension(NumNums, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNums, true);

        // Get the data for the DX Cooling System
        for (DXHeatSysNum = 1; DXHeatSysNum <= NumDXHeatPumpSystems; ++DXHeatSysNum) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     DXHeatSysNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);
            DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpSystemType = CurrentModuleObject; // push Object Name into data array
            DXHeatPumpSystem(DXHeatSysNum).Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                DXHeatPumpSystem(DXHeatSysNum).SchedPtr = ScheduleManager::ScheduleAlwaysOn;
            } else {
                DXHeatPumpSystem(DXHeatSysNum).SchedPtr = GetScheduleIndex(state, Alphas(2));
                if (DXHeatPumpSystem(DXHeatSysNum).SchedPtr == 0) {
                    ShowSevereError(state,
                                    format("{}{}: invalid {} entered ={} for {}={}",
                                           RoutineName,
                                           CurrentModuleObject,
                                           cAlphaFields(2),
                                           Alphas(2),
                                           cAlphaFields(1),
                                           Alphas(1)));
                    state.dataHVACDXHeatPumpSys->ErrorsFound = true;
                }
            }

            if (Util::SameString(Alphas(3), "Coil:Heating:DX:SingleSpeed")) {

                DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType = Alphas(3);
                DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType_Num = CoilDX_HeatingEmpirical;

                DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName = Alphas(4);
            } else if (Util::SameString(Alphas(3), "Coil:Heating:DX:VariableSpeed")) {

                DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType = Alphas(3);
                DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType_Num = Coil_HeatingAirToAirVariableSpeed;

                DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName = Alphas(4);

            } else {
                ShowSevereError(state, format("Invalid entry for {} :{}", cAlphaFields(3), Alphas(3)));
                ShowContinueError(state, format("In {}=\"{}\".", CurrentModuleObject, DXHeatPumpSystem(DXHeatSysNum).Name));
                state.dataHVACDXHeatPumpSys->ErrorsFound = true;
            }

            if (DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
                DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilInletNodeNum =
                    GetCoilInletNodeVariableSpeed(state,
                                                  DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType,
                                                  DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName,
                                                  state.dataHVACDXHeatPumpSys->ErrorsFound);
                DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilOutletNodeNum =
                    GetCoilOutletNodeVariableSpeed(state,
                                                   DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType,
                                                   DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName,
                                                   state.dataHVACDXHeatPumpSys->ErrorsFound);
            } else {
                DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilInletNodeNum = GetCoilInletNode(state,
                                                                                             DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType,
                                                                                             DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName,
                                                                                             state.dataHVACDXHeatPumpSys->ErrorsFound);

                DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilOutletNodeNum = GetCoilOutletNode(state,
                                                                                               DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType,
                                                                                               DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName,
                                                                                               state.dataHVACDXHeatPumpSys->ErrorsFound);
            }

            // Coil air-side outlet node is the control node
            DXHeatPumpSystem(DXHeatSysNum).DXSystemControlNodeNum = DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilOutletNodeNum;

            TestCompSet(state,
                        CurrentModuleObject,
                        DXHeatPumpSystem(DXHeatSysNum).Name,
                        state.dataLoopNodes->NodeID(DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilInletNodeNum),
                        state.dataLoopNodes->NodeID(DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilOutletNodeNum),
                        "Air Nodes");

            ValidateComponent(state,
                              DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType,
                              DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName,
                              IsNotOK,
                              CurrentModuleObject);
            if (IsNotOK) {
                ShowContinueError(state, format("In {} = \"{}\".", CurrentModuleObject, DXHeatPumpSystem(DXHeatSysNum).Name));
                state.dataHVACDXHeatPumpSys->ErrorsFound = true;
            }

            SetUpCompSets(state,
                          DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpSystemType,
                          DXHeatPumpSystem(DXHeatSysNum).Name,
                          DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType,
                          DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName,
                          state.dataLoopNodes->NodeID(DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilInletNodeNum),
                          state.dataLoopNodes->NodeID(DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilOutletNodeNum));

            // Supply air fan operating mode defaulted to constant fan cycling coil/compressor
            DXHeatPumpSystem(DXHeatSysNum).FanOpMode = ContFanCycCoil;

            if (DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType_Num != Coil_HeatingAirToAirVariableSpeed) {
                SetCoilSystemHeatingDXFlag(state, DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilType, DXHeatPumpSystem(DXHeatSysNum).HeatPumpCoilName);
            }

        } // End of the DX System Loop

        if (state.dataHVACDXHeatPumpSys->ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input.  Program terminates.", RoutineName));
        }

        for (DXHeatSysNum = 1; DXHeatSysNum <= NumDXHeatPumpSystems; ++DXHeatSysNum) {
            // Setup Report variables for the DXHeatingSystem that is not reported in the components themselves
            SetupOutputVariable(state,
                                "Coil System Part Load Ratio",
                                Constant::Units::None,
                                DXHeatPumpSystem(DXHeatSysNum).PartLoadFrac,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                DXHeatPumpSystem(DXHeatSysNum).Name);
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }

    // End of Get Input subroutines for the Module
    //******************************************************************************

    // Beginning of Initialization subroutines for the Module
    // *****************************************************************************

    void InitDXHeatPumpSystem(EnergyPlusData &state,
                              int const DXSystemNum,                           // number of the current DX Sys being simulated
                              int const AirLoopNum,                            // number of the current air loop being simulated
                              ObjexxFCL::Optional_int_const OAUnitNum,         // number of the current outdoor air unit being simulated
                              ObjexxFCL::Optional<Real64 const> OAUCoilOutTemp // the coil inlet temperature of OutdoorAirUnit
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith (derived by R.Liesen)
        //       DATE WRITTEN   May 2011
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the DX heat pump Systems.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // REFERENCES:
        // na

        // Using/Aliasing
        bool DoSetPointTest = state.dataHVACGlobal->DoSetPointTest;
        using EMSManager::CheckIfNodeSetPointManagedByEMS;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ControlNode;                // control node number
        Real64 OAUCoilOutletTemp = 0.0; // "ONLY" for zoneHVAC:OutdoorAirUnit

        int NumDXHeatPumpSystems = state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems;

        //  IF (MyOneTimeFlag) THEN
        //    MyOneTimeFlag = .FALSE.
        //  END IF
        if (present(OAUnitNum)) { // This Dx system is component of ZoneHVAC:OutdoorAirUnit
            OAUCoilOutletTemp = OAUCoilOutTemp;
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataHVACDXHeatPumpSys->MySetPointCheckFlag && DoSetPointTest) {
            for (int DXSysIndex = 1; DXSysIndex <= NumDXHeatPumpSystems; ++DXSysIndex) {
                auto &DXHeatPumpSystem = state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXSysIndex);
                ControlNode = DXHeatPumpSystem.DXSystemControlNodeNum;
                if (ControlNode > 0) {
                    if (AirLoopNum == -1) {                                                      // Outdoor Air Unit
                        state.dataLoopNodes->Node(ControlNode).TempSetPoint = OAUCoilOutletTemp; // Set the coil outlet temperature
                    } else {                                                                     // Not an outdoor air unit

                        if (state.dataLoopNodes->Node(ControlNode).TempSetPoint == SensedNodeFlagValue) {
                            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                                ShowSevereError(state,
                                                format("{}: Missing temperature setpoint for DX unit= {}",
                                                       DXHeatPumpSystem.DXHeatPumpSystemType,
                                                       DXHeatPumpSystem.Name));
                                ShowContinueError(state, "  use a Set Point Manager to establish a setpoint at the unit control node.");
                                state.dataHVACGlobal->SetPointErrorFlag = true;
                            } else {
                                CheckIfNodeSetPointManagedByEMS(
                                    state, ControlNode, EMSManager::SPControlType::TemperatureSetPoint, state.dataHVACGlobal->SetPointErrorFlag);
                                if (state.dataHVACGlobal->SetPointErrorFlag) {
                                    ShowSevereError(state,
                                                    format("{}: Missing temperature setpoint for DX unit= {}",
                                                           DXHeatPumpSystem.DXHeatPumpSystemType,
                                                           DXHeatPumpSystem.Name));
                                    ShowContinueError(state, "  use a Set Point Manager to establish a setpoint at the unit control node.");
                                    ShowContinueError(state,
                                                      "  or use an EMS actuator to establish a temperature setpoint at the unit control node.");
                                }
                            }
                        }
                    }
                }
            }
            state.dataHVACDXHeatPumpSys->MySetPointCheckFlag = false;
        }

        // These initializations are done every iteration
        if (AirLoopNum == -1) { // This IF-Then routine is just for ZoneHVAC:OUTDOORAIRUNIT

            state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXSystemNum).DesiredOutletTemp = OAUCoilOutletTemp;

        } else { // Not Outdoor Air Unit
            ControlNode = state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXSystemNum).DXSystemControlNodeNum;
            state.dataHVACDXHeatPumpSys->EconomizerFlag = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive;
            state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXSystemNum).DesiredOutletTemp = state.dataLoopNodes->Node(ControlNode).TempSetPoint;
        }
    }

    // End of Initialization subroutines for the Module
    // *****************************************************************************

    // Beginning of Calculation subroutines for the DXCoolingSystem Module
    // *****************************************************************************

    void ControlDXHeatingSystem(EnergyPlusData &state,
                                int const DXSystemNum,        // index to DXSystem
                                bool const FirstHVACIteration // First HVAC iteration flag
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith (derived from ControlDXSystem by Richard Liesen)
        //       DATE WRITTEN   Jan 2012
        //       MODIFIED       Nov. 2003, R. Raustad, FSEC
        //                      Feb. 2005, M. J. Witte, GARD. Add dehumidification controls and support for multimode DX coil
        //                      Jan. 2008, R. Raustad, FSEC. Added coolreheat to all coil types
        //                      Feb. 2013, B. Shen, ORNL. Add Coil:Heating:DX:VariableSpeed
        //                      Nov. 2016, R. Zhang, LBNL. Applied the coil supply air temperature sensor offset fault model
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //  This subroutine updates the System outlet nodes.

        // METHODOLOGY EMPLOYED:
        //  Data is moved from the System data structure to the System outlet nodes.

        // Using/Aliasing
        using namespace ScheduleManager;
        using DataHVACGlobals::TempControlTol;
        using DXCoils::SimDXCoil;

        using General::SolveRoot;
        using Psychrometrics::PsyHFnTdbW;
        using Psychrometrics::PsyTdpFnWPb;
        using VariableSpeedCoils::SimVariableSpeedCoils;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500);   // Maximum number of iterations for solver
        Real64 constexpr Acc(1.e-3); // Accuracy of solver result

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string CompName; // Name of the DX cooling coil
        Real64 NoOutput;      // Sensible capacity (outlet - inlet) when the compressor is off
        Real64 FullOutput;    // Sensible capacity (outlet - inlet) when the compressor is on
        Real64 ReqOutput;     // Sensible capacity (outlet - inlet) required to meet load or set point temperature
        int InletNode;        // Inlet node number of the DX cooling coil
        int OutletNode;       // Outlet node number of the DX cooling coil
        int ControlNode;      // The node number where a set point is placed to control the DX cooling coil
        Real64 PartLoadFrac;  // The part-load fraction of the compressor

        Real64 DesOutTemp;       // Desired outlet temperature of the DX cooling coil
        Real64 OutletTempDXCoil; // Actual outlet temperature of the DX cooling coil

        bool SensibleLoad; // True if there is a sensible cooling load on this system
        int FanOpMode;     // Supply air fan operating mode
        // added variables to call variable speed DX coils
        int SpeedNum;             // speed number of variable speed DX cooling coil
        Real64 QZnReq;            // Zone load (W), input to variable-speed DX coil
        Real64 QLatReq;           // Zone latent load, input to variable-speed DX coil
        Real64 OnOffAirFlowRatio; // ratio of compressor on flow to average flow over time step
        Real64 TempSpeedOut;      // output at one speed level
        Real64 TempSpeedReqst;    // request capacity at one speed level
        int NumOfSpeeds;          // maximum number of speed
        int VSCoilIndex;          // variable-speed coil index
        int I;                    // interation increment
        Real64 SpeedRatio;        // speed ratio between two neighboring speeds

        auto &DXHeatPumpSystem = state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXSystemNum);

        // Set local variables
        // Retrieve the load on the controlled zone
        OutletNode = DXHeatPumpSystem.DXHeatPumpCoilOutletNodeNum;
        InletNode = DXHeatPumpSystem.DXHeatPumpCoilInletNodeNum;
        ControlNode = DXHeatPumpSystem.DXSystemControlNodeNum;
        DesOutTemp = DXHeatPumpSystem.DesiredOutletTemp;
        CompName = DXHeatPumpSystem.HeatPumpCoilName;
        FanOpMode = DXHeatPumpSystem.FanOpMode;

        PartLoadFrac = 0.0;

        SensibleLoad = false;

        SpeedNum = 1;
        QZnReq = 0.0;
        QLatReq = 0.0;
        OnOffAirFlowRatio = 1.0;
        TempSpeedOut = 0.0;
        TempSpeedReqst = 0.0;
        NumOfSpeeds = 0;
        VSCoilIndex = 0;
        I = 1;
        SpeedRatio = 0.0;

        // If there is a fault of coil SAT Sensor
        if (DXHeatPumpSystem.FaultyCoilSATFlag && (!state.dataGlobal->WarmupFlag) && (!state.dataGlobal->DoingSizing) &&
            (!state.dataGlobal->KickOffSimulation)) {
            // calculate the sensor offset using fault information
            int FaultIndex = DXHeatPumpSystem.FaultyCoilSATIndex;
            DXHeatPumpSystem.FaultyCoilSATOffset = state.dataFaultsMgr->FaultsCoilSATSensor(FaultIndex).CalFaultOffsetAct(state);
            // update the DesOutTemp
            DesOutTemp -= DXHeatPumpSystem.FaultyCoilSATOffset;
        }

        // If DXHeatingSystem is scheduled on and there is flow
        if ((GetCurrentScheduleValue(state, DXHeatPumpSystem.SchedPtr) > 0.0) &&
            (state.dataLoopNodes->Node(InletNode).MassFlowRate > MinAirMassFlow)) {

            // Determine if there is a sensible load on this system
            if ((state.dataLoopNodes->Node(InletNode).Temp < state.dataLoopNodes->Node(ControlNode).TempSetPoint) &&
                (state.dataLoopNodes->Node(InletNode).Temp < DesOutTemp) &&
                (std::abs(state.dataLoopNodes->Node(InletNode).Temp - DesOutTemp) > TempControlTol))
                SensibleLoad = true;

            // If DXHeatingSystem runs with a heating load then set PartLoadFrac on Heating System
            if (SensibleLoad) {
                {
                    Real64 TempOut1;

                    switch (DXHeatPumpSystem.HeatPumpCoilType_Num) {
                    case CoilDX_HeatingEmpirical: { // Coil:Heating:DX:SingleSpeed

                        // Get no load result
                        PartLoadFrac = 0.0;
                        SimDXCoil(state,
                                  CompName,
                                  CompressorOperation::On,
                                  FirstHVACIteration,
                                  DXHeatPumpSystem.HeatPumpCoilIndex,
                                  FanOpMode,
                                  PartLoadFrac);
                        NoOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                                   (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(OutletNode).HumRat) -
                                    PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(OutletNode).HumRat));

                        // Get full load result
                        PartLoadFrac = 1.0;
                        SimDXCoil(state,
                                  CompName,
                                  CompressorOperation::On,
                                  FirstHVACIteration,
                                  DXHeatPumpSystem.HeatPumpCoilIndex,
                                  FanOpMode,
                                  PartLoadFrac);

                        FullOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                                     (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat) -
                                      PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));

                        ReqOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                                    (PsyHFnTdbW(DesOutTemp, state.dataLoopNodes->Node(InletNode).HumRat) -
                                     PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));
                        TempOut1 = state.dataLoopNodes->Node(OutletNode).Temp;
                        //         IF NoOutput is higher than (more heating than required) or very near the ReqOutput, do not run the compressor
                        if ((NoOutput - ReqOutput) > Acc) {
                            PartLoadFrac = 0.0;
                            //         If the FullOutput is greater than (insufficient heating) or very near the ReqOutput,
                            //         run the compressor at PartLoadFrac = 1.
                        } else if ((FullOutput - ReqOutput) < Acc) {
                            PartLoadFrac = 1.0;
                            //         Else find the PLR to meet the load
                        } else {
                            //           OutletTempDXCoil is the full capacity outlet temperature at PartLoadFrac = 1 from the CALL above. If this
                            //           temp is greater than the desired outlet temp, then run the compressor at PartLoadFrac = 1, otherwise find the
                            //           operating PLR.
                            OutletTempDXCoil = state.dataDXCoils->DXCoilOutletTemp(DXHeatPumpSystem.HeatPumpCoilIndex);
                            if (OutletTempDXCoil < DesOutTemp) {
                                PartLoadFrac = 1.0;
                            } else {
                                if (state.dataGlobal->DoCoilDirectSolutions) {
                                    PartLoadFrac = (DesOutTemp - state.dataLoopNodes->Node(InletNode).Temp) /
                                                   (TempOut1 - state.dataLoopNodes->Node(InletNode).Temp);
                                    SimDXCoil(state,
                                              CompName,
                                              CompressorOperation::On,
                                              FirstHVACIteration,
                                              DXHeatPumpSystem.HeatPumpCoilIndex,
                                              FanOpMode,
                                              PartLoadFrac);
                                } else {
                                    int coilIndex = DXHeatPumpSystem.HeatPumpCoilIndex;
                                    auto f = [&state, coilIndex, DesOutTemp](Real64 const PartLoadFrac) {
                                        DXCoils::CalcDXHeatingCoil(state, coilIndex, PartLoadFrac, ContFanCycCoil, 1.0);
                                        Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(coilIndex);
                                        return DesOutTemp - OutletAirTemp;
                                    };
                                    int SolFla = 0;
                                    SolveRoot(state, Acc, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                                    if (SolFla == -1) {
                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (DXHeatPumpSystem.DXCoilSensPLRIter < 1) {
                                                ++DXHeatPumpSystem.DXCoilSensPLRIter;
                                                ShowWarningError(
                                                    state,
                                                    format("{} - Iteration limit exceeded calculating DX unit sensible part-load ratio for unit = {}",
                                                           DXHeatPumpSystem.DXHeatPumpSystemType,
                                                           DXHeatPumpSystem.Name));
                                                ShowContinueError(state, format("Estimated part-load ratio  = {:.3R}", (ReqOutput / FullOutput)));
                                                ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                                ShowContinueErrorTimeStamp(
                                                    state,
                                                    "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                            } else {
                                                ShowRecurringWarningErrorAtEnd(state,
                                                                               DXHeatPumpSystem.DXHeatPumpSystemType + " \"" + DXHeatPumpSystem.Name +
                                                                                   "\" - Iteration limit exceeded calculating sensible part-load "
                                                                                   "ratio error continues. Sensible "
                                                                                   "PLR statistics follow.",
                                                                               DXHeatPumpSystem.DXCoilSensPLRIterIndex,
                                                                               PartLoadFrac,
                                                                               PartLoadFrac);
                                            }
                                        }
                                    } else if (SolFla == -2) {
                                        PartLoadFrac = ReqOutput / FullOutput;
                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (DXHeatPumpSystem.DXCoilSensPLRFail < 1) {
                                                ++DXHeatPumpSystem.DXCoilSensPLRFail;
                                                ShowWarningError(state,
                                                                 format("{} - DX unit sensible part-load ratio calculation failed: part-load ratio "
                                                                        "limits exceeded, for unit = {}",
                                                                        DXHeatPumpSystem.DXHeatPumpSystemType,
                                                                        DXHeatPumpSystem.Name));
                                                ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                                ShowContinueErrorTimeStamp(
                                                    state,
                                                    "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                            } else {
                                                ShowRecurringWarningErrorAtEnd(
                                                    state,
                                                    DXHeatPumpSystem.DXHeatPumpSystemType + " \"" + DXHeatPumpSystem.Name +
                                                        "\" - DX unit sensible part-load ratio calculation failed error continues. Sensible PLR "
                                                        "statistics follow.",
                                                    DXHeatPumpSystem.DXCoilSensPLRFailIndex,
                                                    PartLoadFrac,
                                                    PartLoadFrac);
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (PartLoadFrac > 1.0) {
                            PartLoadFrac = 1.0;
                        } else if (PartLoadFrac < 0.0) {
                            PartLoadFrac = 0.0;
                        }
                    } break;
                    case Coil_HeatingAirToAirVariableSpeed: {
                        // variable-speed air-to-air heating coil, begin -------------------------
                        // Get no load result
                        PartLoadFrac = 0.0;
                        SpeedNum = 1;
                        QZnReq = 0.0;
                        QLatReq = 0.0;
                        OnOffAirFlowRatio = 1.0;
                        SpeedRatio = 0.0;

                        SimVariableSpeedCoils(state,
                                              CompName,
                                              DXHeatPumpSystem.HeatPumpCoilIndex,
                                              FanOpMode,
                                              CompressorOperation::On,
                                              PartLoadFrac,
                                              SpeedNum,
                                              SpeedRatio,
                                              QZnReq,
                                              QLatReq,
                                              OnOffAirFlowRatio);

                        VSCoilIndex = DXHeatPumpSystem.HeatPumpCoilIndex;
                        NumOfSpeeds = state.dataVariableSpeedCoils->VarSpeedCoil(VSCoilIndex).NumOfSpeeds;

                        NoOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                                   (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(OutletNode).HumRat) -
                                    PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(OutletNode).HumRat));

                        // Get full load result
                        PartLoadFrac = 1.0;

                        SpeedNum = NumOfSpeeds;
                        SpeedRatio = 1.0;
                        QZnReq = 0.001; // to indicate the coil is running
                        SimVariableSpeedCoils(state,
                                              CompName,
                                              VSCoilIndex,
                                              FanOpMode,
                                              CompressorOperation::On,
                                              PartLoadFrac,
                                              SpeedNum,
                                              SpeedRatio,
                                              QZnReq,
                                              QLatReq,
                                              OnOffAirFlowRatio);

                        FullOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                                     (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat) -
                                      PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));

                        ReqOutput = state.dataLoopNodes->Node(InletNode).MassFlowRate *
                                    (PsyHFnTdbW(DesOutTemp, state.dataLoopNodes->Node(InletNode).HumRat) -
                                     PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, state.dataLoopNodes->Node(InletNode).HumRat));
                        //         IF NoOutput is higher than (more heating than required) or very near the ReqOutput, do not run the compressor
                        if ((NoOutput - ReqOutput) > Acc) {
                            PartLoadFrac = 0.0;
                            SpeedNum = 1;
                            SpeedRatio = 0.0;
                            //         If the FullOutput is greater than (insufficient heating) or very near the ReqOutput,
                            //         run the compressor at PartLoadFrac = 1.
                        } else if ((FullOutput - ReqOutput) < Acc) {
                            PartLoadFrac = 1.0;
                            SpeedNum = NumOfSpeeds;
                            SpeedRatio = 1.0;
                            //         Else find the PLR to meet the load
                        } else {
                            //           OutletTempDXCoil is the full capacity outlet temperature at PartLoadFrac = 1 from the CALL above. If this
                            //           temp is greater than the desired outlet temp, then run the compressor at PartLoadFrac = 1, otherwise find the
                            //           operating PLR.
                            OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(VSCoilIndex).OutletAirDBTemp;
                            if (OutletTempDXCoil < DesOutTemp) {
                                PartLoadFrac = 1.0;
                                SpeedNum = NumOfSpeeds;
                                SpeedRatio = 1.0;
                            } else {
                                PartLoadFrac = 1.0;
                                SpeedNum = 1;
                                SpeedRatio = 1.0;
                                QZnReq = 0.001; // to indicate the coil is running
                                SimVariableSpeedCoils(state,
                                                      CompName,
                                                      VSCoilIndex,
                                                      FanOpMode,
                                                      CompressorOperation::On,
                                                      PartLoadFrac,
                                                      SpeedNum,
                                                      SpeedRatio,
                                                      QZnReq,
                                                      QLatReq,
                                                      OnOffAirFlowRatio);

                                TempSpeedOut = state.dataVariableSpeedCoils->VarSpeedCoil(VSCoilIndex).OutletAirDBTemp;

                                if ((TempSpeedOut - DesOutTemp) < Acc) {
                                    // Check to see which speed to meet the load
                                    PartLoadFrac = 1.0;
                                    SpeedRatio = 1.0;
                                    TempOut1 = TempSpeedOut;
                                    for (I = 2; I <= NumOfSpeeds; ++I) {
                                        SpeedNum = I;
                                        SimVariableSpeedCoils(state,
                                                              CompName,
                                                              VSCoilIndex,
                                                              FanOpMode,
                                                              CompressorOperation::On,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              OnOffAirFlowRatio);

                                        TempSpeedOut = state.dataVariableSpeedCoils->VarSpeedCoil(VSCoilIndex).OutletAirDBTemp;

                                        if ((TempSpeedOut - DesOutTemp) > Acc) {
                                            SpeedNum = I;
                                            break;
                                        }
                                        TempOut1 = TempSpeedOut;
                                    }
                                    if (state.dataGlobal->DoCoilDirectSolutions) {
                                        SpeedRatio = (DesOutTemp - TempOut1) / (TempSpeedOut - TempOut1);
                                        SimVariableSpeedCoils(state,
                                                              CompName,
                                                              VSCoilIndex,
                                                              FanOpMode,
                                                              CompressorOperation::On,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              OnOffAirFlowRatio);
                                    } else {
                                        auto f = [&state, VSCoilIndex, DesOutTemp, SpeedNum, FanOpMode](Real64 const x) {
                                            return VSCoilSpeedResidual(state, x, VSCoilIndex, DesOutTemp, SpeedNum, FanOpMode);
                                        };
                                        int SolFla = 0;
                                        General::SolveRoot(state, Acc, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);

                                        if (SolFla == -1) {
                                            if (!state.dataGlobal->WarmupFlag) {
                                                if (DXHeatPumpSystem.DXCoilSensPLRIter < 1) {
                                                    ++DXHeatPumpSystem.DXCoilSensPLRIter;
                                                    ShowWarningError(state,
                                                                     format("{} - Iteration limit exceeded calculating DX unit sensible part-load "
                                                                            "ratio for unit = {}",
                                                                            DXHeatPumpSystem.DXHeatPumpSystemType,
                                                                            DXHeatPumpSystem.Name));
                                                    ShowContinueError(state, format("Estimated part-load ratio  = {:.3R}", (ReqOutput / FullOutput)));
                                                    ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                                    ShowContinueErrorTimeStamp(
                                                        state,
                                                        "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(
                                                        state,
                                                        DXHeatPumpSystem.DXHeatPumpSystemType + " \"" + DXHeatPumpSystem.Name +
                                                            "\" - Iteration limit exceeded calculating sensible part-load ratio error continues. "
                                                            "Sensible PLR statistics follow.",
                                                        DXHeatPumpSystem.DXCoilSensPLRIterIndex,
                                                        PartLoadFrac,
                                                        PartLoadFrac);
                                                }
                                            }
                                        } else if (SolFla == -2) {
                                            PartLoadFrac = ReqOutput / FullOutput;
                                            if (!state.dataGlobal->WarmupFlag) {
                                                if (DXHeatPumpSystem.DXCoilSensPLRFail < 1) {
                                                    ++DXHeatPumpSystem.DXCoilSensPLRFail;
                                                    ShowWarningError(state,
                                                                     format("{} - DX unit sensible part-load ratio calculation failed: part-load "
                                                                            "ratio limits exceeded, for unit = {}",
                                                                            DXHeatPumpSystem.DXHeatPumpSystemType,
                                                                            DXHeatPumpSystem.Name));
                                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                                    ShowContinueErrorTimeStamp(
                                                        state,
                                                        "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(
                                                        state,
                                                        DXHeatPumpSystem.DXHeatPumpSystemType + " \"" + DXHeatPumpSystem.Name +
                                                            "\" - DX unit sensible part-load ratio calculation failed error continues. Sensible PLR "
                                                            "statistics follow.",
                                                        DXHeatPumpSystem.DXCoilSensPLRFailIndex,
                                                        PartLoadFrac,
                                                        PartLoadFrac);
                                                }
                                            }
                                        }
                                    }
                                } else {
                                    if (state.dataGlobal->DoCoilDirectSolutions) {
                                        PartLoadFrac = (DesOutTemp - state.dataLoopNodes->Node(InletNode).Temp) /
                                                       (TempSpeedOut - state.dataLoopNodes->Node(InletNode).Temp);
                                        SimVariableSpeedCoils(state,
                                                              CompName,
                                                              VSCoilIndex,
                                                              FanOpMode,
                                                              CompressorOperation::On,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              OnOffAirFlowRatio);
                                    } else {
                                        auto f = [&state, VSCoilIndex, DesOutTemp, FanOpMode](Real64 const x) {
                                            return VSCoilCyclingResidual(state, x, VSCoilIndex, DesOutTemp, FanOpMode);
                                        };
                                        int SolFla = 0;
                                        General::SolveRoot(state, Acc, MaxIte, SolFla, PartLoadFrac, f, 1.0e-10, 1.0);
                                        if (SolFla == -1) {
                                            if (!state.dataGlobal->WarmupFlag) {
                                                if (DXHeatPumpSystem.DXCoilSensPLRIter < 1) {
                                                    ++DXHeatPumpSystem.DXCoilSensPLRIter;
                                                    ShowWarningError(state,
                                                                     format("{} - Iteration limit exceeded calculating DX unit sensible part-load "
                                                                            "ratio for unit = {}",
                                                                            DXHeatPumpSystem.DXHeatPumpSystemType,
                                                                            DXHeatPumpSystem.Name));
                                                    ShowContinueError(state, format("Estimated part-load ratio  = {:.3R}", (ReqOutput / FullOutput)));
                                                    ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                                    ShowContinueErrorTimeStamp(
                                                        state,
                                                        "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(
                                                        state,
                                                        DXHeatPumpSystem.DXHeatPumpSystemType + " \"" + DXHeatPumpSystem.Name +
                                                            "\" - Iteration limit exceeded calculating sensible part-load ratio error continues. "
                                                            "Sensible PLR statistics follow.",
                                                        DXHeatPumpSystem.DXCoilSensPLRIterIndex,
                                                        PartLoadFrac,
                                                        PartLoadFrac);
                                                }
                                            }
                                        } else if (SolFla == -2) {
                                            PartLoadFrac = ReqOutput / FullOutput;
                                            if (!state.dataGlobal->WarmupFlag) {
                                                if (DXHeatPumpSystem.DXCoilSensPLRFail < 1) {
                                                    ++DXHeatPumpSystem.DXCoilSensPLRFail;
                                                    ShowWarningError(state,
                                                                     format("{} - DX unit sensible part-load ratio calculation failed: part-load "
                                                                            "ratio limits exceeded, for unit = {}",
                                                                            DXHeatPumpSystem.DXHeatPumpSystemType,
                                                                            DXHeatPumpSystem.Name));
                                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                                    ShowContinueErrorTimeStamp(
                                                        state,
                                                        "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                                } else {
                                                    ShowRecurringWarningErrorAtEnd(
                                                        state,
                                                        DXHeatPumpSystem.DXHeatPumpSystemType + " \"" + DXHeatPumpSystem.Name +
                                                            "\" - DX unit sensible part-load ratio calculation failed error continues. Sensible PLR "
                                                            "statistics follow.",
                                                        DXHeatPumpSystem.DXCoilSensPLRFailIndex,
                                                        PartLoadFrac,
                                                        PartLoadFrac);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        if (PartLoadFrac > 1.0) {
                            PartLoadFrac = 1.0;
                        } else if (PartLoadFrac < 0.0) {
                            PartLoadFrac = 0.0;
                        }
                    } break;
                    default: {
                        ShowFatalError(state,
                                       format("ControlDXHeatingSystem: Invalid DXHeatPumpSystem coil type = {}", DXHeatPumpSystem.HeatPumpCoilType));
                    } break;
                    }
                }
            } // End of cooling load type (sensible or latent) if block
        }     // End of If DXheatingSystem is scheduled on and there is flow

        // Set the final results
        DXHeatPumpSystem.PartLoadFrac = PartLoadFrac;
        DXHeatPumpSystem.SpeedRatio = SpeedRatio;
        DXHeatPumpSystem.SpeedNum = SpeedNum;
    }

    //******************************************************************************

    Real64 VSCoilCyclingResidual(EnergyPlusData &state,
                                 Real64 const PartLoadRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                 int CoilIndex,
                                 Real64 desiredTemp,
                                 int FanOpMode)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Bo Shen
        //       DATE WRITTEN   Feb, 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function, iterate part-load ratio
        //  compare the desired temperature value with exit temperature from a variable-speed heating coil

        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                  "",
                                                  CoilIndex,
                                                  FanOpMode,
                                                  CompressorOperation::On,
                                                  PartLoadRatio,
                                                  state.dataHVACDXHeatPumpSys->SpeedNum,
                                                  state.dataHVACDXHeatPumpSys->SpeedRatio,
                                                  state.dataHVACDXHeatPumpSys->QZnReqr,
                                                  state.dataHVACDXHeatPumpSys->QLatReqr,
                                                  state.dataHVACDXHeatPumpSys->OnandOffAirFlowRatio);

        Real64 OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(CoilIndex).OutletAirDBTemp;
        return desiredTemp - OutletAirTemp;
    }

    //******************************************************************************

    Real64 VSCoilSpeedResidual(EnergyPlusData &state,
                               Real64 const SpeedRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                               int CoilIndex,
                               Real64 desiredTemp,
                               int speedNumber,
                               int FanOpMode)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Bo Shen
        //       DATE WRITTEN   Feb, 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function, iterate speed ratio
        //  compare the desired temperature value with exit temperature from a variable-speed heating coil
        state.dataHVACDXHeatPumpSys->SpeedNumber = speedNumber;
        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                  "",
                                                  CoilIndex,
                                                  FanOpMode,
                                                  CompressorOperation::On,
                                                  state.dataHVACDXHeatPumpSys->SpeedPartLoadRatio,
                                                  state.dataHVACDXHeatPumpSys->SpeedNumber,
                                                  SpeedRatio,
                                                  state.dataHVACDXHeatPumpSys->QZoneReq,
                                                  state.dataHVACDXHeatPumpSys->QLatentReq,
                                                  state.dataHVACDXHeatPumpSys->AirFlowOnOffRatio);
        Real64 OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(CoilIndex).OutletAirDBTemp;
        return desiredTemp - OutletAirTemp;
    }

    int GetHeatingCoilInletNodeNum(EnergyPlusData &state, std::string const &DXHeatCoilSysName, bool &InletNodeErrFlag)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu, FSEC
        //       DATE WRITTEN   Apr. 2019
        // PURPOSE OF THIS SUBROUTINE:
        // Get inlet node number

        if (state.dataHVACDXHeatPumpSys->GetInputFlag) { // First time subroutine has been entered
            GetDXHeatPumpSystemInput(state);
            state.dataHVACDXHeatPumpSys->GetInputFlag = false;
        }

        int NodeNum = 0;
        if (state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems > 0) {
            int DXHeatSysNum = Util::FindItemInList(DXHeatCoilSysName, state.dataHVACDXHeatPumpSys->DXHeatPumpSystem);
            if (DXHeatSysNum > 0 && DXHeatSysNum <= state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems) {
                NodeNum = state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilInletNodeNum;
            }
        }
        if (NodeNum == 0) InletNodeErrFlag = true;

        return NodeNum;
    }

    int GetHeatingCoilOutletNodeNum(EnergyPlusData &state, std::string const &DXHeatCoilSysName, bool &OutletNodeErrFlag)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu, FSEC
        //       DATE WRITTEN   Apr. 2019
        // PURPOSE OF THIS SUBROUTINE:
        // Get Outlet node number

        if (state.dataHVACDXHeatPumpSys->GetInputFlag) { // First time subroutine has been entered
            GetDXHeatPumpSystemInput(state);
            state.dataHVACDXHeatPumpSys->GetInputFlag = false;
        }

        int NodeNum = 0;
        if (state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems > 0) {
            int DXHeatSysNum = Util::FindItemInList(DXHeatCoilSysName, state.dataHVACDXHeatPumpSys->DXHeatPumpSystem);
            if (DXHeatSysNum > 0 && DXHeatSysNum <= state.dataHVACDXHeatPumpSys->NumDXHeatPumpSystems) {
                NodeNum = state.dataHVACDXHeatPumpSys->DXHeatPumpSystem(DXHeatSysNum).DXHeatPumpCoilOutletNodeNum;
            }
        }
        if (NodeNum == 0) OutletNodeErrFlag = true;

        return NodeNum;
    }

} // namespace HVACDXHeatPumpSystem

} // namespace EnergyPlus