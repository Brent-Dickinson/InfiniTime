# Heart Rate Subsystem -- Orientation Guide

This document provides a high-level map of the PineTime heart rate
subsystem.\
It is intended as a fast re-entry reference, not a deep algorithm
document.

------------------------------------------------------------------------

# 1. Architectural Overview

The heart rate feature follows a vertical slice:

    UI (HeartRate screen)
            ↓
    HeartRateController
            ↓
    HeartRateTask (FreeRTOS task)
            ↓
    Ppg (signal processing)
            ↓
    Hrs3300 driver (sensor)

BLE integration runs alongside this path via:

    HeartRateController → HeartRateService (BLE)

------------------------------------------------------------------------

# 2. File Roles

## 2.1 UI Layer

### `displayapp/screens/HeartRate.cpp`

-   LVGL-based UI screen
-   Displays BPM and status
-   Start/Stop button calls:
    -   `heartRateController.Enable()`
    -   `heartRateController.Disable()`
-   Refresh task runs periodically (\~100 ms)
-   Pulls state and BPM from controller

This file never touches hardware directly.

------------------------------------------------------------------------

## 2.2 Controller Layer

### `components/heartrate/HeartRateController.h/.cpp`

Central coordination object.

Responsibilities: - Stores: - Current heart rate value - Measurement
state - Holds pointers to: - `HeartRateTask` - `HeartRateService`
(BLE) - Provides: - `Enable()` - `Disable()` - `HeartRate()` -
`State()` - Called by: - UI - HeartRateTask (to update BPM)

The controller does not: - Sample hardware - Run signal processing

It is a state container and router.

------------------------------------------------------------------------

## 2.3 Task Layer (RTOS Boundary)

### `heartratetask/HeartRateTask.h/.cpp`

FreeRTOS task that owns the measurement state machine.

Responsibilities: - Enable / disable sensor - Manage measurement
modes: - Disabled - Waiting - ForegroundMeasuring -
BackgroundMeasuring - Receive sensor data - Call `Ppg::Process()` -
Update controller with computed BPM

This is where: - Sampling timing happens - Sensor data flow is
controlled - Crashes due to bad instrumentation are most likely

If modifying sampling behavior, start here.

------------------------------------------------------------------------

## 2.4 Signal Processing

### `components/heartrate/Ppg.h/.cpp`

Pure signal-processing layer.

Responsibilities: - Accept raw PPG samples - Filter / buffer data - Run
FFT-based heart rate extraction - Return computed BPM

Does not: - Know about UI - Know about BLE - Manage hardware

If implementing HRV, this is the layer that will likely expand.

------------------------------------------------------------------------

## 2.5 Sensor Driver

### `drivers/Hrs3300.h/.cpp`

Low-level I²C interface to the HRS3300 optical sensor.

Responsibilities: - Configure registers - Enable / disable sensor - Read
raw sample data

This is the hardware boundary.

------------------------------------------------------------------------

## 2.6 BLE Service

### `components/ble/HeartRateService.h/.cpp`

Implements standard BLE Heart Rate Service.

Responsibilities: - Expose BPM via GATT - Notify subscribers

Receives data from `HeartRateController`.

------------------------------------------------------------------------

# 3. Data Flow Summary

When user presses Start:

1.  UI calls `HeartRateController.Enable()`
2.  Controller instructs `HeartRateTask`
3.  Task enables sensor
4.  Sensor produces samples
5.  Task feeds samples into `Ppg`
6.  `Ppg` computes BPM
7.  Task updates controller
8.  Controller updates:
    -   UI (via polling)
    -   BLE service (notifications)

------------------------------------------------------------------------

# 4. Important Boundaries

## UI must remain passive

-   Only reads state
-   Never drives sampling timing directly

## Controller must remain lightweight

-   State holder
-   No heavy computation

## Task owns timing

-   All sampling frequency instrumentation should live here
-   Safest place to insert diagnostics

## Ppg should stay pure

-   No logging
-   No RTOS calls
-   No BLE references

------------------------------------------------------------------------

# 5. Where to Modify for HRV

If building HRV support:

-   Raw beat timing extraction → `Ppg`
-   Sampling rate instrumentation → `HeartRateTask`
-   RR interval exposure → possibly Controller + new BLE service
-   UI visualization → new screen or extension of HeartRate screen

Do not insert timing logic into: - UI - BLE service - Controller (except
as data holder)

------------------------------------------------------------------------

# 6. Mental Model

Think of the subsystem as:

-   Driver = hardware
-   Task = real-time owner
-   Ppg = math
-   Controller = state + routing
-   UI/BLE = presentation

If debugging crashes: - Check task context first - Then stack size -
Then sampling logic
