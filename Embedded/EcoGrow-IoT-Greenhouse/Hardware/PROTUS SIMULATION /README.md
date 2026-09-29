# Proteus Simulation

## Overview

The EcoGrow system was also tested using **Proteus Design Suite** to verify the circuit connections, microcontroller operation, sensor interfacing, and control logic in a simulated environment.

Proteus simulation helps validate the embedded system design before or alongside physical hardware implementation.

## Simulation Objectives

* Verify circuit connections
* Test microcontroller-based control logic
* Simulate sensor interfacing
* Verify relay control operation
* Test greenhouse automation logic
* Identify hardware and firmware issues during development

## Simulated System

The Proteus simulation can include the following major elements:

* Arduino UNO
* Temperature and humidity sensing
* Air-quality sensing
* Soil-moisture sensing
* Water-level sensing
* Light sensing
* Relay-controlled devices
* Cooling fan
* Exhaust fan
* Water pump
* Lighting system
* LCD/display, if used in the simulation

## Simulation Flow

```text
Sensors
   ↓
Arduino UNO
   ↓
Sensor Data Processing
   ↓
Decision / Control Logic
   ↓
Relay Control
   ↓
Greenhouse Actuators
```

## Verification

The simulation was used to verify:

* Sensor input handling
* Threshold-based control
* Relay switching
* Automatic actuator control
* Microcontroller logic
* Overall system behavior

## Files

The actual Proteus simulation files and screenshots will be added to this folder.

Typical files may include:

```text
EcoGrow_Proteus.pdsprj
EcoGrow_Proteus.pds
Simulation_Schematic.png
Simulation_Result.png
```

## Note

Proteus simulation represents the virtual validation of the system. Final hardware behavior may vary depending on actual sensor characteristics, component tolerances, power supply conditions, and physical implementation.
