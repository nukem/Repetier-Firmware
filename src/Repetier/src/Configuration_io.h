/*

This file defines io solutions used. This is the lowest level and is the base
for all higher level functions using io operations. At several places we need
subsets of these list of operations. To make configuration easy and easy to 
understand, we use a technique called "x macros". This requires that only
predefined macro names for IO are used here. Do NOT add anything else here
or compilation/functionality will break.

Rules:
1. Each definition will create a class that is named like the first parameter.
This class is later used as input to templates building higher functions. By
convention the names should start with IO followed by something that helps you
identify the function. 
2. Do not use a semicolon at the end. Macro definition gets different meanings
and will add the semicolon if required.

*/

/* Define motor pins here. Each motor needs a setp, dir and enable pin. */

ENDSTOP_NONE(endstopNone)
// For use when no output is wanted, but possible
IO_OUTPUT_FAKE(fakeOut)

// X Motor

IO_OUTPUT(IOX1Step, ORIG_X_STEP_PIN)
IO_OUTPUT_INVERTED(IOX1Dir, ORIG_X_DIR_PIN)
IO_OUTPUT_INVERTED(IOX1Enable, ORIG_X_ENABLE_PIN)

// Y Motor

IO_OUTPUT(IOY1Step, ORIG_Y_STEP_PIN)
IO_OUTPUT(IOY1Dir, ORIG_Y_DIR_PIN)
IO_OUTPUT_INVERTED(IOY1Enable, ORIG_Y_ENABLE_PIN)

// Z Motor

IO_OUTPUT(IOZ1Step, ORIG_Z_STEP_PIN)
IO_OUTPUT_INVERTED(IOZ1Dir, ORIG_Z_DIR_PIN)
IO_OUTPUT_INVERTED(IOZ1Enable, ORIG_Z_ENABLE_PIN)

// E0 Motor

IO_OUTPUT(IOE1Step, ORIG_E0_STEP_PIN)
IO_OUTPUT_INVERTED(IOE1Dir, ORIG_E0_DIR_PIN)
IO_OUTPUT_INVERTED(IOE1Enable, ORIG_E0_ENABLE_PIN)

// E1 Motor

IO_OUTPUT(IOE2Step, ORIG_E1_STEP_PIN)
IO_OUTPUT(IOE2Dir, ORIG_E1_DIR_PIN)
IO_OUTPUT_INVERTED(IOE2Enable, ORIG_E1_ENABLE_PIN)

// Autolevel Motor 1

//IO_OUTPUT(IOAL1Step, 51)
//IO_OUTPUT(IOAL1Dir, 53)
//IO_OUTPUT_INVERTED(IOAL1Enable, 49)

// Autolevel Motor 1

//IO_OUTPUT(IOAL2Step, 39)
//IO_OUTPUT(IOAL2Dir, 13)
//IO_OUTPUT_INVERTED(IOAL2Enable, 40)

// Define your endstops inputs

IO_INPUT(IOEndstopXMin, ORIG_X_MIN_PIN)
IO_INPUT(IOEndstopYMin, ORIG_Y_MIN_PIN)
IO_INPUT(IOEndstopZMin, ORIG_Z_MIN_PIN)
IO_INPUT_INVERTED_PULLUP(IOZProbe, ORIG_ZPROBE_PIN)

IO_INPUT(IOJam1, FIL_JAM_SENSOR_PIN1)
IO_INPUT(IOJam2, FIL_JAM_SENSOR_PIN2)
IO_INPUT_INVERTED(IOFilamentDetect1, FIL_SENSOR1_PIN1)
IO_INPUT_INVERTED(IOFilamentDetect2, FIL_SENSOR1_PIN2)

/**
IO_INPUT_LOG(IOJam1_log, IOJam1, true)
IO_INPUT_LOG(IOJam2_log, IOJam2, true)
IO_INPUT_LOG(IOFilamentDetect1_log, IOFilamentDetect1, false)
IO_INPUT_LOG(IOFilamentDetect2_log, IOFilamentDetect2, false)
**/

// Case Light
#if defined(CASE_LIGHT_PIN) && CASE_LIGHT_PIN > 0
IO_OUTPUT(caseLightPin, CASE_LIGHT_PIN)
LIGHT_STATE_MONOCHROME(caseLightState)
LIGHT_COND(caseLightState, true, Printer::caseLightMode, 255, 255, 255, 255)
LIGHT_COND(caseLightState, true, !GUI::isGUIIdle(), 255, 255, 255, 255)
LIGHT_COND(caseLightState, Printer::isUIErrorMessage(), LIGHT_STATE_BLINK_SLOW, 255, 255, 255, 255)
LIGHT_COND(caseLightState, Printer::isDoorOpen(), LIGHT_STATE_ON, 255, 255, 255, 255)
LIGHT_SOURCE_MONOCHROME(caseLightDriver, caseLightPin, caseLightState)
#endif

#if defined(UI_BACKLIGHT_PIN) && UI_BACKLIGHT_PIN > 0
IO_OUTPUT(controllerBackLightPin, UI_BACKLIGHT_PIN)
LIGHT_STATE_MONOCHROME(controllerBackLightState)
LIGHT_COND(controllerBackLightState, true, !GUI::isGUIIdle(), 255, 255, 255, 255)
LIGHT_SOURCE_MONOCHROME(controllerBackLightDriver, controllerBackLightPin, controllerBackLightState)
#endif

// Controller input pins

#if defined(UI_ENCODER_CLICK) && UI_ENCODER_CLICK >= 0
IO_INPUT_INVERTED(ControllerClick, UI_ENCODER_CLICK)
#else
IO_INPUT_DUMMY(ControllerClick, false)
#endif

#if defined(UI_ENCODER_A) && UI_ENCODER_A >= 0
IO_INPUT_INVERTED_PULLUP(ControllerEncA, UI_ENCODER_A)
#else
IO_INPUT_DUMMY(ControllerEncA, false)
#endif

#if defined(UI_ENCODER_B) && UI_ENCODER_B >= 0
IO_INPUT_INVERTED_PULLUP(ControllerEncB, UI_ENCODER_B)
#else
IO_INPUT_DUMMY(ControllerEncB, false)
#endif

#if defined(UI_BACK_PIN) && UI_BACK_PIN >= 0
IO_INPUT_INVERTED(ControllerBack, UI_BACK_PIN)
#else
IO_INPUT_DUMMY(ControllerBack, false)
#endif

#if defined(UI_RESET_PIN) && UI_RESET_PIN >= 0
IO_INPUT_INVERTED(ControllerReset, UI_RESET_PIN)
#else
IO_INPUT_DUMMY(ControllerReset, false)
#endif

IO_INPUT_INVERTED(ControllerPrevious, UI_PREV_PIN)
IO_INPUT_INVERTED(ControllerNext, UI_NEXT_PIN)

// Define our endstops solutions
// You need to define all min and max endstops for all
// axes except E even if you have none!
/**
ENDSTOP_SWITCH_HW(endstopXMin, IOEndstopXMin, X_AXIS, -1)
ENDSTOP_NONE(endstopXMax)
ENDSTOP_SWITCH_HW(endstopYMin, IOEndstopYMin, Y_AXIS, -1)
ENDSTOP_NONE(endstopYMax)
ENDSTOP_SWITCH_HW(endstopZMin, IOEndstopZMin, Z_AXIS, -1)
ENDSTOP_NONE(endstopZMax)
**/

// Define fans

IO_OUTPUT(IOFan1, ORIG_FAN_PIN)
IO_OUTPUT(IOFan2, ORIG_FAN2_PIN)
IO_PWM_SOFTWARE(Fan1PWM, IOFan1, 0)
IO_PWM_SOFTWARE(Fan2PWM, IOFan2, 0)
//IO_PWM_HARDWARE(Fan1PWM, 37,5000)
// IO_PDM_SOFTWARE(Fan1NoKSPWM, IOFan1) // alternative to PWM signals
//IO_PWM_KICKSTART(Fan1PWM, Fan1NoKSPWM, 20, 85)
// For debugging - reports new values and then calls real pwm
// IO_PWM_REPORT(Fan1Report, Fan1PWM)
// Define temperature sensors

// Typically they require an analog input (12 bit) so define
// them first.

IO_ANALOG_INPUT(IOAnalogBed1, TEMP_1_PIN, 5)
IO_ANALOG_INPUT(IOAnalogExt1, TEMP_0_PIN, 5)
IO_ANALOG_INPUT(IOAnalogExt2, TEMP_2_PIN, 5)

// Need a conversion table for epcos NTC
#define NUM_Ntc_custom1 33
#define Ntc_custom1 \
    {294, 2560},{375, 2400},{461, 2272},{526, 2184},{601, 2104},{674, 2048},{811, 1928},{925, 1848},{1071, 1768},{1219, 1696},{1445, 1584},{1673, 1496},{1902, 1416},{2187, 1312},{2454, 1224},{2672, 1152},{3000, 1048},{3196, 968},{3439, 864},{3583, 784},{3736, 680},{3899, 536},{3960, 448},{4024, 336},{4067, 192},{4079, 156},{4100, 0},{4120, -160}

#define NUM_Ntc_custom2 19
#define Ntc_custom2 \
    {628,1280},{859,1200},{1113,1120},{1382,1040},{1660,960},{1938,880},{2211,800},{2473,720},{2718,640},{2945,560},{3148,480},{3328,400},{3482,320},{3613,240},{3722,160},{3815,80},{3895,0},{3972,-80},{4055,-160}

IO_TEMP_TABLE_NTC(TempTableExtruder, Ntc_custom1)
IO_TEMP_TABLE_NTC(TempTableBed, Ntc_custom2)
// Now create the temperature inputs

IO_TEMPERATURE_TABLE(TempBed1, IOAnalogBed1, TempTableBed)
IO_TEMPERATURE_TABLE(TempExt1, IOAnalogExt1, TempTableExtruder)
IO_TEMPERATURE_TABLE(TempExt2, IOAnalogExt2, TempTableExtruder)

// Use PWM outputs to heat. If using hardware PWM make sure
// that the selected pin can be used as hardware pwm otherwise
// select a software pwm model whcih works on all pins.

#if MOTHERBOARD == 405
IO_PWM_HARDWARE(PWMExtruder1, HEATER_0_PIN, 1000)
IO_PWM_HARDWARE(PWMExtruder2, HEATER_2_PIN, 1000)
IO_PWM_HARDWARE(PWMBed1, HEATER_1_PIN, 1000)
#else
//IO_OUTPUT(IOExtr1, HEATER_0_PIN)
//IO_OUTPUT(IOExtr2, HEATER_2_PIN)
IO_OUTPUT(IOBed1, HEATER_1_PIN)
IO_PWM_HARDWARE(PWMExtruder1, HEATER_0_PIN, 60)
IO_PWM_HARDWARE(PWMExtruder2, HEATER_2_PIN, 60)
IO_PWM_SOFTWARE(PWMBed1, IOBed1, 0)
#endif
// IO_OUTPUT(IOCooler1, FAN2_PIN)
// IO_PWM_SOFTWARE(PWMCoolerExt1, FAN2_PIN, 0)

ENDSTOP_SWITCH(endstopXMin, IOEndstopXMin)
ENDSTOP_NONE(endstopXMax)
ENDSTOP_SWITCH(endstopYMin, IOEndstopYMin)
ENDSTOP_NONE(endstopYMax)
ENDSTOP_SWITCH(endstopZMin, IOEndstopZMin)
ENDSTOP_NONE(endstopZMax)

// Define all stepper motors used
STEPPER_SIMPLE(XMotor, IOX1Step, IOX1Dir, IOX1Enable, endstopXMin, endstopNone)
STEPPER_SIMPLE(YMotor, IOY1Step, IOY1Dir, IOY1Enable, endstopYMin, endstopNone)
STEPPER_SIMPLE(ZMotor, IOZ1Step, IOZ1Dir, IOZ1Enable, endstopZMin, endstopNone)
STEPPER_SIMPLE(E1MotorBase, IOE1Step, IOE1Dir, IOE1Enable, endstopNone, endstopNone)
STEPPER_OBSERVEABLE(E1Motor, E1MotorBase)
STEPPER_SIMPLE(E2MotorBase, IOE2Step, IOE2Dir, IOE2Enable, endstopNone, endstopNone)
STEPPER_OBSERVEABLE(E2Motor, E2MotorBase)
//STEPPER_SIMPLE(AL1Motor, IOAL1Step, IOAL1Dir, IOAL1Enable, endstopNone, endstopNone)
//STEPPER_SIMPLE(AL2Motor, IOAL2Step, IOAL2Dir, IOAL2Enable, endstopNone, endstopNone)

// Servos
#if ORIG_SERVO_PIN > -1
IO_OUTPUT(Servo1Pin, ORIG_SERVO_PIN)
SERVO_ANALOG(Servo1, 0, Servo1Pin, 500, 2500, 1050)
#endif

// Heat manages are used for every component that needs to
// control temperature. Higher level classes take these as input
// and simple heater like a heated bed use it directly.
// HEAT_MANAGER_PID(name, tp, index, input, output, maxTemp, maxPwm, sampleTime, decVariance, decPeriod, p, i, d, driveMin, driveMax, hotPlugable)
HEAT_MANAGER_PID(HeatedBed1, 'B', 0, TempBed1, PWMBed1, 120, 255, 1000, 5, 30000, 24.4, 4.84, 30.87, 80, 255, true)
HEAT_MANAGER_PID(HeaterExtruder1, 'E', 0, TempExt1, PWMExtruder1, 260, 255, 1000, 10, 20000, 6.03, 0.34, 71.33, 40, 220, false)
HEAT_MANAGER_PID(HeaterExtruder2, 'E', 1, TempExt2, PWMExtruder2, 260, 255, 1000, 10, 20000, 4.15, 0.20, 56.80, 40, 220, false)

// HEAT_MANAGER_DYN_DEAD_TIME(HeaterExtruder1, 'E', 0, TempExt1, PWMExtruder1, 260, 255, 100, 10, 20000, 150, 7, 7, 200, 7, 7, false)
// HEAT_MANAGER_DYN_DEAD_TIME(HeaterExtruder2, 'E', 1, TempExt2, PWMExtruder2, 260, 255, 100, 10, 20000, 150, 7, 7, 200, 7, 7, false)

// Coolers are stand alone functions that allow it to control
// a fan with external sensors. Many extruders require a cooling
// fan pointer to the extruder to prevent heat rising up.
// These can be controlled by the cooler. Since it is
// independent you just tell what part needs cooling.
// Other use cases are board cooling and heated chambers.
COOLER_MANAGER_HEATER(Ext1Cooler, HeaterExtruder1, Fan1PWM, 50, 160, 10, 255)
COOLER_MANAGER_HEATER(Ext2Cooler, HeaterExtruder2, Fan2PWM, 50, 160, 10, 255)
// Define tools. They get inserted into a tool array in configuration.h
// Typical tools are:
// TOOL_EXTRUDER(name, offx, offy, offz, heater, stepper, resolution, yank, maxSpeed, acceleration, advance, startScript, endScript)

TOOL_EXTRUDER(ToolExtruder1, 0, 0, 0, HeaterExtruder1, /*AL1Motor */ E1Motor, 1.75, 147.0, 5, 30, 5000, 40, "", "", nullptr)
TOOL_EXTRUDER(ToolExtruder2, 0, 0, 0, HeaterExtruder2, /*AL2Motor */ E2Motor, 1.75, 147.0, 5, 30, 5000, 40, "", "", nullptr)
//TOOL_LASER(Laser3, 0, 0, 0, Fan1NoKSPWM, fakeOut, fakeOut, 3000, 1, 100, 150.0, 1.5, "", "")
//TOOL_CNC(CNC4, 0, 0, 0, Fan1NoKSPWM, fakeOut, fakeOut, fakeOut, 7000, 3000, "", "")

// Use a signal that changes while extruder moves
JAM_DETECTOR_HW(JamExtruder1, E1Motor, IOJam1, ToolExtruder1, 400, 10, 500)
JAM_DETECTOR_HW(JamExtruder2, E2Motor, IOJam2, ToolExtruder2, 400, 10, 500)

// Use a signal that is high, when filament is loaded
FILAMENT_DETECTOR(FilamentDetector1, IOFilamentDetect1, ToolExtruder1)
FILAMENT_DETECTOR(FilamentDetector2, IOFilamentDetect2, ToolExtruder2)

// Beeper
#if BEEPER_PIN > -1
IO_OUTPUT(IOBeeperMain, BEEPER_PIN)
BEEPER_SOURCE_IO(MainBeeper, IOBeeperMain)
#endif