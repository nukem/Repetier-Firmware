/*
    This file is part of Repetier-Firmware.

    Repetier-Firmware is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Repetier-Firmware is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Repetier-Firmware.  If not, see <http://www.gnu.org/licenses/>.

    This firmware is a nearly complete rewrite of the sprinter firmware
    by kliment (https://github.com/kliment/Sprinter)
    which based on Tonokip RepRap firmware rewrite based off of Hydra-mmm firmware.
*/

/**

Coordinate system transformations:

Level 1: G-code => Coordinates like send via g-codes.

Level 2: Real coordinates => Coordinates corrected by coordinate shift via G92
         currentPosition and lastCmdPos are from this level.
Level 3: Transformed and shifted => Include extruder offset and bed rotation.
         These variables are only stored temporary.

Level 4: Step position => Level 3 converted into steps for motor position
        currentPositionSteps and destinationPositionSteps are from this level.

Level 5: Nonlinear motor step position, only for nonlinear drive systems
         destinationDeltaSteps


*/

#ifndef PRINTER_H_INCLUDED
#define PRINTER_H_INCLUDED

#if defined(AUTOMATIC_POWERUP) && AUTOMATIC_POWERUP && PS_ON_PIN > -1
#define ENSURE_POWER \
    { Printer::enablePowerIfNeeded(); }
#else
#undef AUTOMATIC_POWERUP
#define AUTOMATIC_POWERUP 0
#define ENSURE_POWER \
    {}
#endif

union floatLong {
    float f;
    uint32_t l;
#ifdef SUPPORT_64_BIT_MATH
    uint64_t L;
#endif
};

union wizardVar {
    float f;
    int32_t l;
    uint32_t ul;
    int16_t i;
    uint16_t ui;
    int8_t c;
    uint8_t uc;

    wizardVar()
        : i(0) {}
    wizardVar(float _f)
        : f(_f) {}
    wizardVar(int32_t _f)
        : l(_f) {}
    wizardVar(uint32_t _f)
        : ul(_f) {}
    wizardVar(int16_t _f)
        : i(_f) {}
    wizardVar(uint16_t _f)
        : ui(_f) {}
    wizardVar(int8_t _f)
        : c(_f) {}
    wizardVar(uint8_t _f)
        : uc(_f) {}
};

#define FIRMWARE_EVENT_JAM_DEBUG 1

class FirmwareEvent {
public:
    int eventId;
    wizardVar param1, param2;
    static FirmwareEvent eventList[4];
    static volatile fast8_t start, length;

    // Add event to queue if possible
    static bool queueEvent(int id, wizardVar p1, wizardVar p2);
    // Resolve all event tasks
    static void handleEvents();
};

#define PRINTER_FLAG0_STEPPER_DISABLED 1
#define PRINTER_FLAG0_SEPERATE_EXTRUDER_INT 2
#define PRINTER_FLAG0_TEMPSENSOR_DEFECT 4
#define PRINTER_FLAG0_FORCE_CHECKSUM 8
#define PRINTER_FLAG0_MANUAL_MOVE_MODE 16
#define PRINTER_FLAG0_ZPROBEING 64
// #define PRINTER_FLAG0_LARGE_MACHINE 128
#define PRINTER_FLAG1_HOMED_ALL 1
#define PRINTER_FLAG1_AUTOMOUNT 2
#define PRINTER_FLAG1_ANIMATION 4
#define PRINTER_FLAG1_ALLKILLED 8
#define PRINTER_FLAG1_UI_ERROR_MESSAGE 16
#define PRINTER_FLAG1_NO_DESTINATION_CHECK 32
#define PRINTER_FLAG1_POWER_ON 64
#define PRINTER_FLAG1_ALLOW_COLD_EXTRUSION 128
#define PRINTER_FLAG2_BLOCK_RECEIVING 1
#define PRINTER_FLAG2_AUTORETRACT 2
#define PRINTER_FLAG2_RESET_FILAMENT_USAGE 4
#define PRINTER_FLAG2_IGNORE_M106_COMMAND 8
#define PRINTER_FLAG2_DEBUG_JAM 16
#define PRINTER_FLAG2_JAMCONTROL_DISABLED 32
#define PRINTER_FLAG2_HOMING 64
#define PRINTER_FLAG2_ALL_E_MOTORS 128 // Set all e motors flag
#define PRINTER_FLAG3_PRINTING 8       // set explicitly with M530
#define PRINTER_FLAG3_AUTOREPORT_TEMP 16
#define PRINTER_FLAG3_SUPPORTS_STARTSTOP 32
#define PRINTER_FLAG3_DOOR_OPEN 64

// List of possible interrupt events (1-255 allowed)
#define PRINTER_INTERRUPT_EVENT_JAM_DETECTED 1
#define PRINTER_INTERRUPT_EVENT_JAM_SIGNAL0 2
#define PRINTER_INTERRUPT_EVENT_JAM_SIGNAL1 3
#define PRINTER_INTERRUPT_EVENT_JAM_SIGNAL2 4
#define PRINTER_INTERRUPT_EVENT_JAM_SIGNAL3 5
#define PRINTER_INTERRUPT_EVENT_JAM_SIGNAL4 6
#define PRINTER_INTERRUPT_EVENT_JAM_SIGNAL5 7
// define an integer number of steps more than large enough to get to end stop from anywhere
#define HOME_DISTANCE_STEPS (Printer::zMaxSteps - Printer::zMinSteps + 1000)
#define HOME_DISTANCE_MM (HOME_DISTANCE_STEPS * invAxisStepsPerMM[Z_AXIS])
// Some defines to make clearer reading, as we overload these Cartesian memory locations for delta
#define towerAMaxSteps Printer::xMaxSteps
#define towerBMaxSteps Printer::yMaxSteps
#define towerCMaxSteps Printer::zMaxSteps
#define towerAMinSteps Printer::xMinSteps
#define towerBMinSteps Printer::yMinSteps
#define towerCMinSteps Printer::zMinSteps

class Plane {
public:
    // f(x, y) = ax + by + c
    float a, b, c;
    float z(float x, float y) {
        return a * x + y * b + c;
    }
};

#ifndef DEFAULT_PRINTER_MODE
#if NUM_TOOLS > 0
#define DEFAULT_PRINTER_MODE PRINTER_MODE_FFF
#elif defined(SUPPORT_LASER) && SUPPORT_LASER
#define DEFAULT_PRINTER_MODE PRINTER_MODE_LASER
#elif defined(SUPPORT_CNC) && SUPPORT_CNC
#define DEFAULT_PRINTER_MODE PRINTER_MODE_CNC
#else
#error No supported printer mode compiled
#endif
#endif

extern bool runBedLeveling(int save); // save = S parameter in gcode

/**
The Printer class is the main class for the control of the 3d printer. Here all
movement related key variables are stored like positions, accelerations.

## Coordinates

The firmware works with 4 different coordinate systems and understanding the
dependencies between them is crucial to a good understanding on how positions
are handled.

### Real world floating coordinates (RWC)

These coordinates are the real floating positions with any offsets subtracted,
which might be set with G92. This is used to show coordinates or for computations
based on real positions. Any correction coming from rotation or distortion is
not included in these coordinates. currentPosition and lastCmdPos use this coordinate
system.

When these coordinates need to be used for computation, the value of offsetX, offsetY and offsetZ
is always added. These are the offsets of the currently active tool to virtual tool center
(normally first extruder).

### Rotated floating coordinates (ROTC)

If auto leveling is active, printing to the official coordinates is not possible. We have to assume
that the bed is somehow rotated against the Cartesian mechanics from the printer. Applying
_transformToPrinter_ to the real world coordinates, rotates them around the origin to
be equal to the rotated bed. _transformFromPrinter_ would apply the opposite transformation.

### Cartesian motor position coordinates (CMC)

The position of motors is stored as steps from 0. The reason for this is that it is crucial that
no rounding errors ever cause addition of any steps. These positions are simply computed by
multiplying the ROTC coordinates with the axisStepsPerMM.

If distortion correction is enabled, there is an additional factor for the z position that
gets added: _zCorrectionStepsIncluded_ This value is recalculated by every move added to
reflect the distortion at any given xyz position.

### Nonlinear motor position coordinates (NMC)

In case of a nonlinear mechanic like a delta printer, the CMC does not define the motor positions.
An additional transformation converts the CMC coordinates into NMC.

### Transformations from RWC to CMC

Given:
- Target position for tool: x_rwc, y_rwc, z_rwc
- Tool offsets: offsetX, offsetY, offsetZ
- Offset from bed leveling: Motion1::zprobeZOffset

Step 1: Convert to ROTC

    transformToPrinter(x_rwc + Motion1::toolOffset[X_AXIS], y_rwc + Motion1::toolOffset[Y_AXIS], z_rwc +  Motion1::toolOffset[Z_AXIS], x_rotc, y_rotc, z_rotc);
    z_rotc += Motion1::zprobeZOffset

Step 2: Compute CMC

    x_cmc = static_cast<int32_t>(floor(x_rotc * axisStepsPerMM[X_AXIS] + 0.5f));
    y_cmc = static_cast<int32_t>(floor(y_rotc * axisStepsPerMM[Y_AXIS] + 0.5f));
    z_cmc = static_cast<int32_t>(floor(z_rotc * axisStepsPerMM[Z_AXIS] + 0.5f));

### Transformation from CMC to RWC

Note: _zCorrectionStepsIncluded_ comes from distortion correction and gets set when a move is queued by the queuing function.
Therefore it is not visible in the inverse transformation above. When transforming back, consider if the value was set or not!

Step 1: Convert to ROTC

    x_rotc = static_cast<float>(x_cmc) * invAxisStepsPerMM[X_AXIS];
    y_rotc = static_cast<float>(y_cmc) * invAxisStepsPerMM[Y_AXIS];
    #if NONLINEAR_SYSTEM
    z_rotc = static_cast<float>(z_cmc * invAxisStepsPerMM[Z_AXIS] - Motion1::zprobeZOffset;
    #else
    z_rotc = static_cast<float>(z_cmc - zCorrectionStepsIncluded) * invAxisStepsPerMM[Z_AXIS] - Motion1::zprobeZOffset;
    #endif

Step 2: Convert to RWC

    transformFromPrinter(x_rotc, y_rotc, z_rotc,x_rwc, y_rwc, z_rwc);
    x_rwc -= Motion1::toolOffset[X_AXIS]; // Offset from active extruder or z probe
    y_rwc -= Motion1::toolOffset[Y_AXIS];
    z_rwc -= Motion1::toolOffset[Z_AXIS];
*/
class Printer {
    static uint8_t debugLevel;

public:
    static uint16_t menuMode;
    static float homingFeedrate[]; // Feedrate in mm/s for homing.
    // static uint32_t maxInterval; // slowest allowed interval
    static uint8_t relativeCoordinateMode;         ///< Determines absolute (false) or relative Coordinates (true).
    static uint8_t relativeExtruderCoordinateMode; ///< Determines Absolute or Relative E Codes while in Absolute Coordinates mode. E is always relative in Relative Coordinates mode.

    static uint8_t unitIsInches;
    static uint8_t flag0, flag1; // 1 = stepper disabled, 2 = use external extruder interrupt, 4 = temp Sensor defect, 8 = homed
    static uint8_t flag2, flag3;
    static uint32_t interval;   ///< Last step duration in ticks.
    static uint32_t timer;      ///< used for acceleration/deceleration timing
    static uint32_t stepNumber; ///< Step number in current move.
    static millis_t lastTempReport;
    static int32_t printingTime;       ///< Printing time in seconds
    static float extrudeMultiplyError; ///< Accumulated error during extrusion
    static float extrusionFactor;      ///< Extrusion multiply factor
#if DRIVE_SYSTEM != DELTA || defined(DOXYGEN)
    static int32_t zCorrectionStepsIncluded;
#endif
#if FEATURE_Z_PROBE || MAX_HARDWARE_ENDSTOP_Z || NONLINEAR_SYSTEM || defined(DOXYGEN)
    static int32_t stepsRemainingAtZHit;
#endif
#if FEATURE_AUTOLEVEL || defined(DOXYGEN)
    static float autolevelTransformation[9]; ///< Transformation matrix
#endif
#if FAN_THERMO_PIN > -1 || defined(DOXYGEN)
    static float thermoMinTemp;
    static float thermoMaxTemp;
#endif
#if FEATURE_BABYSTEPPING || defined(DOXYGEN)
    static int16_t zBabystepsMissing;
    static int16_t zBabysteps;
#endif
    static float feedrate;               ///< Last requested feedrate.
    static int feedrateMultiply;         ///< Multiplier for feedrate in percent (factor 1 = 100)
    static unsigned int extrudeMultiply; ///< Flow multiplier in percent (factor 1 = 100)
    static uint8_t interruptEvent;       ///< Event generated in interrupts that should/could be handled in main thread
    static speed_t vMaxReached;          ///< Maximum reached speed
    static uint32_t msecondsPrinting;    ///< Milliseconds of printing time (means time with heated extruder)
    static float filamentPrinted;        ///< mm of filament printed since counting started
    static float filamentPrintedTotal;   ///< Total amount of filament printed in meter
#if ENABLE_BACKLASH_COMPENSATION || defined(DOXYGEN)
    static float backlashX;
    static float backlashY;
    static float backlashZ;
    static uint8_t backlashDir;
#endif
    // Print status related
    static int currentLayer;
    static int maxLayer;       // -1 = unknown
    static char printName[21]; // max. 20 chars + 0
    static float progress;
    static fast8_t breakLongCommand; // Set by M108 to stop long tasks
    static fast8_t wizardStackPos;
    static wizardVar wizardStack[WIZARD_STACK_SIZE];

    static void handleInterruptEvent();

    static INLINE void setInterruptEvent(uint8_t evt, bool highPriority) {
        if (highPriority || interruptEvent == 0)
            interruptEvent = evt;
    }
    static INLINE void setMenuMode(uint16_t mode, bool on) {
        if (on)
            menuMode |= mode;
        else
            menuMode &= ~mode;
    }

    static INLINE bool isMenuMode(uint8_t mode) {
        return (menuMode & mode) == mode;
    }
    static void setDebugLevel(uint8_t newLevel);
    static void toggleEcho();
    static void toggleInfo();
    static void toggleErrors();
    static void toggleDryRun();
    static void toggleCommunication();
    static void toggleNoMoves();
    static void toggleEndStop();
    static INLINE uint8_t getDebugLevel() {
        return debugLevel;
    }
    static INLINE bool debugEcho() {
        return ((debugLevel & 1) != 0);
    }

    static INLINE bool debugInfo() {
        return ((debugLevel & 2) != 0);
    }

    static INLINE bool debugErrors() {
        return ((debugLevel & 4) != 0);
    }

    static INLINE bool debugDryrun() {
        return ((debugLevel & 8) != 0);
    }

    static INLINE bool debugCommunication() {
        return ((debugLevel & 16) != 0);
    }

    static INLINE bool debugNoMoves() {
        return ((debugLevel & 32) != 0);
    }

    static INLINE bool debugEndStop() {
        return ((debugLevel & 64) != 0);
    }

    static INLINE bool debugFlag(uint8_t flags) {
        return (debugLevel & flags);
    }

    static INLINE void debugSet(uint8_t flags) {
        setDebugLevel(debugLevel | flags);
    }

    static INLINE void debugReset(uint8_t flags) {
        setDebugLevel(debugLevel & ~flags);
    }
#if AUTOMATIC_POWERUP
    static void enablePowerIfNeeded();
#endif
    /** Sets the pwm for the fan speed. Gets called by motion control or Commands::setFanSpeed. */
    static void setFanSpeedDirectly(uint8_t speed, int fanId);

    /** For large machines, the nonlinear transformation can exceed integer 32bit range, so floating point math is needed. */

    static INLINE uint8_t isAdvanceActivated() {
        return flag0 & PRINTER_FLAG0_SEPERATE_EXTRUDER_INT;
    }

    static INLINE void setAdvanceActivated(uint8_t b) {
        flag0 = (b ? flag0 | PRINTER_FLAG0_SEPERATE_EXTRUDER_INT : flag0 & ~PRINTER_FLAG0_SEPERATE_EXTRUDER_INT);
    }

    static INLINE uint8_t isHomedAll() {
        return flag1 & PRINTER_FLAG1_HOMED_ALL;
    }

    static INLINE void unsetHomedAll() {
        flag1 &= ~PRINTER_FLAG1_HOMED_ALL;
    }

    static INLINE void setHomedAll(bool b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_HOMED_ALL : flag1 & ~PRINTER_FLAG1_HOMED_ALL);
    }

    static INLINE uint8_t isAutoreportTemp() {
        return flag3 & PRINTER_FLAG3_AUTOREPORT_TEMP;
    }

    static INLINE void setAutoreportTemp(uint8_t b) {
        flag3 = (b ? flag3 | PRINTER_FLAG3_AUTOREPORT_TEMP : flag3 & ~PRINTER_FLAG3_AUTOREPORT_TEMP);
    }

    static INLINE uint8_t isAllKilled() {
        return flag1 & PRINTER_FLAG1_ALLKILLED;
    }

    static INLINE void setAllKilled(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_ALLKILLED : flag1 & ~PRINTER_FLAG1_ALLKILLED);
    }

    static INLINE uint8_t isAutomount() {
        return flag1 & PRINTER_FLAG1_AUTOMOUNT;
    }

    static INLINE void setAutomount(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_AUTOMOUNT : flag1 & ~PRINTER_FLAG1_AUTOMOUNT);
    }

    static INLINE uint8_t isAnimation() {
        return flag1 & PRINTER_FLAG1_ANIMATION;
    }

    static INLINE void setAnimation(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_ANIMATION : flag1 & ~PRINTER_FLAG1_ANIMATION);
    }

    static INLINE uint8_t isUIErrorMessage() {
        return flag1 & PRINTER_FLAG1_UI_ERROR_MESSAGE;
    }

    static INLINE void setUIErrorMessage(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_UI_ERROR_MESSAGE : flag1 & ~PRINTER_FLAG1_UI_ERROR_MESSAGE);
    }

    static INLINE uint8_t isNoDestinationCheck() {
        return flag1 & PRINTER_FLAG1_NO_DESTINATION_CHECK;
    }

    static INLINE void setNoDestinationCheck(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_NO_DESTINATION_CHECK : flag1 & ~PRINTER_FLAG1_NO_DESTINATION_CHECK);
    }

    static INLINE uint8_t isPowerOn() {
        return flag1 & PRINTER_FLAG1_POWER_ON;
    }

    static INLINE void setPowerOn(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_POWER_ON : flag1 & ~PRINTER_FLAG1_POWER_ON);
    }

    static INLINE uint8_t isColdExtrusionAllowed() {
        return flag1 & PRINTER_FLAG1_ALLOW_COLD_EXTRUSION;
    }

    static INLINE void setColdExtrusionAllowed(uint8_t b) {
        flag1 = (b ? flag1 | PRINTER_FLAG1_ALLOW_COLD_EXTRUSION : flag1 & ~PRINTER_FLAG1_ALLOW_COLD_EXTRUSION);
        if (b)
            Com::printFLN(PSTR("Cold extrusion allowed"));
        else
            Com::printFLN(PSTR("Cold extrusion disallowed"));
    }

    static INLINE uint8_t isBlockingReceive() {
        return flag2 & PRINTER_FLAG2_BLOCK_RECEIVING;
    }

    static INLINE void setBlockingReceive(uint8_t b) {
        flag2 = (b ? flag2 | PRINTER_FLAG2_BLOCK_RECEIVING : flag2 & ~PRINTER_FLAG2_BLOCK_RECEIVING);
        Com::printFLN(b ? Com::tPauseCommunication : Com::tContinueCommunication);
    }

    static INLINE uint8_t isAutoretract() {
        return flag2 & PRINTER_FLAG2_AUTORETRACT;
    }

    static INLINE void setAutoretract(uint8_t b) {
        flag2 = (b ? flag2 | PRINTER_FLAG2_AUTORETRACT : flag2 & ~PRINTER_FLAG2_AUTORETRACT);
        Com::printFLN(PSTR("Autoretract:"), b);
    }

    static INLINE uint8_t isPrinting() {
        return flag3 & PRINTER_FLAG3_PRINTING;
    }

    static INLINE void setPrinting(uint8_t b) {
        flag3 = (b ? flag3 | PRINTER_FLAG3_PRINTING : flag3 & ~PRINTER_FLAG3_PRINTING);
        Printer::setMenuMode(MENU_MODE_PRINTING, b);
    }

    static INLINE uint8_t isStartStopSupported() {
        return flag3 & PRINTER_FLAG3_SUPPORTS_STARTSTOP;
    }

    static INLINE void setSupportStartStop(uint8_t b) {
        flag3 = (b ? flag3 | PRINTER_FLAG3_SUPPORTS_STARTSTOP : flag3 & ~PRINTER_FLAG3_SUPPORTS_STARTSTOP);
    }

    static INLINE uint8_t isDoorOpen() {
        return (flag3 & PRINTER_FLAG3_DOOR_OPEN) != 0;
    }

    static bool updateDoorOpen();

    static INLINE uint8_t isHoming() {
        return flag2 & PRINTER_FLAG2_HOMING;
    }

    static INLINE void setHoming(uint8_t b) {
        flag2 = (b ? flag2 | PRINTER_FLAG2_HOMING : flag2 & ~PRINTER_FLAG2_HOMING);
    }
    static INLINE uint8_t isAllEMotors() {
        return flag2 & PRINTER_FLAG2_ALL_E_MOTORS;
    }

    static INLINE void setAllEMotors(uint8_t b) {
        flag2 = (b ? flag2 | PRINTER_FLAG2_ALL_E_MOTORS : flag2 & ~PRINTER_FLAG2_ALL_E_MOTORS);
    }

    static INLINE uint8_t isDebugJam() {
        return (flag2 & PRINTER_FLAG2_DEBUG_JAM) != 0;
    }

    static INLINE uint8_t isDebugJamOrDisabled() {
        return (flag2 & (PRINTER_FLAG2_DEBUG_JAM | PRINTER_FLAG2_JAMCONTROL_DISABLED)) != 0;
    }

    static INLINE void setDebugJam(uint8_t b) {
        flag2 = (b ? flag2 | PRINTER_FLAG2_DEBUG_JAM : flag2 & ~PRINTER_FLAG2_DEBUG_JAM);
        Com::printFLN(PSTR("Jam debugging:"), b);
    }

    static INLINE uint8_t isJamcontrolDisabled() {
        return (flag2 & PRINTER_FLAG2_JAMCONTROL_DISABLED) != 0;
    }

    static INLINE void setJamcontrolDisabled(uint8_t b) {
        // TODO: add jam control
#if EXTRUDER_JAM_CONTROL
        //if (b)
        //    Extruder::markAllUnjammed();
#endif
        flag2 = (b ? flag2 | PRINTER_FLAG2_JAMCONTROL_DISABLED : flag2 & ~PRINTER_FLAG2_JAMCONTROL_DISABLED);
        Com::printFLN(PSTR("Jam control disabled:"), b);
    }

    static INLINE void toggleAnimation() {
        setAnimation(!isAnimation());
    }
    static INLINE float convertToMM(float x) {
        return (unitIsInches ? x * 25.4 : x);
    }
    static INLINE bool areAllSteppersDisabled() {
        return flag0 & PRINTER_FLAG0_STEPPER_DISABLED;
    }
    static INLINE void setAllSteppersDiabled() {
        flag0 |= PRINTER_FLAG0_STEPPER_DISABLED;
    }
    static INLINE void unsetAllSteppersDisabled() {
        flag0 &= ~PRINTER_FLAG0_STEPPER_DISABLED;
    }
    static INLINE bool isAnyTempsensorDefect() {
        return (flag0 & PRINTER_FLAG0_TEMPSENSOR_DEFECT);
    }
    static INLINE void setAnyTempsensorDefect() {
        flag0 |= PRINTER_FLAG0_TEMPSENSOR_DEFECT;
        debugSet(8);
    }
    static INLINE void unsetAnyTempsensorDefect() {
        flag0 &= ~PRINTER_FLAG0_TEMPSENSOR_DEFECT;
    }
    static INLINE bool isManualMoveMode() {
        return (flag0 & PRINTER_FLAG0_MANUAL_MOVE_MODE);
    }
    static INLINE void setManualMoveMode(bool on) {
        flag0 = (on ? flag0 | PRINTER_FLAG0_MANUAL_MOVE_MODE : flag0 & ~PRINTER_FLAG0_MANUAL_MOVE_MODE);
    }
    static INLINE void setZProbingActive(bool on) {
        flag0 = (on ? flag0 | PRINTER_FLAG0_ZPROBEING : flag0 & ~PRINTER_FLAG0_ZPROBEING);
    }
    static INLINE bool isZProbingActive() {
        return (flag0 & PRINTER_FLAG0_ZPROBEING);
    }

    /** \brief copies currentPosition to parameter. */
    static void realPosition(float& xp, float& yp, float& zp);

    static INLINE void insertStepperHighDelay() {
#if STEPPER_HIGH_DELAY > 0
        HAL::delayMicroseconds(STEPPER_HIGH_DELAY);
#endif
    }
    static void updateDerivedParameter();
    /** If we are not homing or destination check being disabled, this will reduce _destinationSteps_ to a
    valid value. In other words this works as software endstop. */
    static void constrainDestinationCoords();
    /** \brief Sets the destination coordinates to values stored in com.

    Extracts x,y,z,e,f from g-code considering active units. Converted result is stored in currentPosition and lastCmdPos. Converts
    position to destinationSteps including rotation and offsets, excluding distortion correction (which gets added on move queuing).
    \param com g-code with new destination position.
     */
    static void setDestinationStepsFromGCode(GCode* com);
    /** \brief Move to position without considering transformations.

    Computes the destinationSteps without rotating but including active offsets!
    The coordinates are in printer coordinates with no G92 offset.

    \param x Target x coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target y coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target z coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target e coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target f feedrate or IGNORE_COORDINATE if it should use latest feedrate.
    \return true if queuing was successful.
    */
    static uint8_t moveTo(float x, float y, float z, float e, float f);
    /** \brief Move to position considering transformations.

    Computes the destinationSteps including rotating and active offsets.
    The coordinates are in printer coordinates with no G92 offset.

    \param x Target x coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target y coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target z coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target e coordinate or IGNORE_COORDINATE if it should be ignored.
    \param x Target f feedrate or IGNORE_COORDINATE if it should use latest feedrate.
    \param pathOptimize true if path planner should include it in calculation, otherwise default start/end speed is enforced.
    \return true if queuing was successful.
    */
    static uint8_t moveToReal(float x, float y, float z, float e, float f, bool pathOptimize = true);
    static void kill(uint8_t only_steppers);
    static void setup();
    static void defaultLoopActions();
    static void setOrigin(float xOff, float yOff, float zOff);
    static int getFanSpeed(int fanId);
    static void setFanSpeed(int speed, bool immediately, int fanId);

#if MAX_HARDWARE_ENDSTOP_Z || defined(DOXYGEN)
    static float runZMaxProbe();
#endif
#if FEATURE_Z_PROBE || defined(DOXYGEN)
    static bool startProbing(bool runScript, bool enforceStartHeight = true);
    static void finishProbing();
    static float runZProbe(bool first, bool last, uint8_t repeat = Z_PROBE_REPETITIONS, bool runStartScript = true, bool enforceStartHeight = true);
    static void measureZProbeHeight(float curHeight);
    static void waitForZProbeStart();
    static float bendingCorrectionAt(float x, float y);
#endif
    static void zBabystep();

    static INLINE void resetWizardStack() {
        wizardStackPos = 0;
    }
    static INLINE void pushWizardVar(wizardVar v) {
        wizardStack[wizardStackPos++] = v;
    }
    static INLINE wizardVar popWizardVar() {
        return wizardStack[--wizardStackPos];
    }
    static void showConfiguration();
    static void setCaseLight(bool on);
    static void reportCaseLightStatus();
#if JSON_OUTPUT || defined(DOXYGEN)
    static void showJSONStatus(int type);
#endif
    static void homeXAxis();
    static void homeYAxis();
    static void homeZAxis();
    static void pausePrint();
    static void continuePrint();
    static void stopPrint();
#if FEATURE_Z_PROBE || defined(DOXYGEN)
    /** \brief Prepares printer for probing commands.

    Probing can not start under all conditions. This command therefore makes sure,
    a probing command can be executed by:
    - Ensuring all axes are homed.
    - Going to a low z position for fast measuring.
    - Go to a position, where enabling the z-probe is possible without leaving the valid print area.
    */
    static void prepareForProbing();
#endif
};

#endif // PRINTER_H_INCLUDED