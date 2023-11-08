/*
 * General constants
 *
 * Copyright © - 2022 - Intermode, Inc
 */
#ifndef __CONSTS_HPP__
#define __CONSTS_HPP__


#include <cstdint>

static const uint8_t kucNumBitsPerByte = 8;     ///< Number of bits per byte

static const uint32_t kulHourToMinute = 60;     ///< Minutes per hour
static const uint32_t kulSecondToMicrosecond = 1000000;     ///< Microseconds per second

static const uint32_t kulKilometerToMillimeter = 1000000;   ///< Millimeters per kilometer


namespace intermode {

#define UNUSED(x) ((void)(x))   ///< Suppress unused variable warnings

/*******************************************************************************
 * Enumerations
 ******************************************************************************/
enum ReturnCodes {
    keReturnNormal = 0,     // Execution successful
    keReturnError,          // Generic error
    keReturnInvalid,        // Invalid parameter
    keReturnTimeout,        // Timeout error
    keReturnFull,           // Target object full
    keReturnMemory,         // Memory error
    keReturnReinit,         // Target object needs reinitialization
    keReturnNotInitialized, // Target object not initialized
    keReturnNotReady,       // Target object not ready
    keReturnNotFound,       // Target object not found
    keReturnVersion,        // Version mismatch
    keReturnSize,           // Size mismatch
    keReturnName,           // Name invalid
    keReturnPermission,     // Incorrect permissions

    // NVS-specific return codes
    keReturnPartitionNotFound,  // NVS partition not found
    keReturnNamespaceNotFound,  // NVS namespace not found
};

// Relay/contactor states
// Must be a boolean type
enum RelayState {
    keStateOpen = 0,
    keStateClosed = 1,
};

enum class Position {
    kePositionRight = 0,
    kePositionLeft = 1,
    kePositionFront = 2,
    kePositionRear = 3,
};

enum PrndGear {
    kePrndGearPark = 0,
    kePrndGearReverse = 1,
    kePrndGearNeutral = 2,
    kePrndGearDrive = 3,
    kePrndGearEstopSlow = 4,
    kePrndGearEstopFast = 5,
};

enum DriveMode {
    keDriveModeFour = 0,
    keDriveModeFront = 1,
    keDriveModeRear = 2,
    keDriveModeIndependentAccel = 3,
    keDriveModeIndependentKph = 4,
    keDriveModeIndependentRpm = 5,
    keDriveModeIndependentEncoderAbs = 6,
    keDriveModeIndependentEncoderRel = 7,
    keDriveModeIndependentCurrent = 8,
};

enum SteerMode {
    keSteerModeFront = 0,
    keSteerModeRear = 1,
    keSteerModeFour = 2,
    keSteerModeCrab = 3,
    keSteerModeIndependent = 4,
};

} /* namespace intermode */

#endif
