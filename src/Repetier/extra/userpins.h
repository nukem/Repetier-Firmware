/*****************************************************************
* Davinci 2.0A Duo
* 
******************************************************************/
#define CPU_ARCH    ARCH_ARM

#define ORIG_X_STEP_PIN 15
#define ORIG_X_DIR_PIN 14
#define ORIG_X_MIN_PIN 11
#define ORIG_X_MAX_PIN -1
#define ORIG_X_ENABLE_PIN 29

#define ORIG_Y_STEP_PIN 30
#define ORIG_Y_DIR_PIN 12
#define ORIG_Y_MIN_PIN 68
#define ORIG_Y_MAX_PIN -1
#define ORIG_Y_ENABLE_PIN 69

#define ORIG_Z_STEP_PIN 119
#define ORIG_Z_DIR_PIN 118
#define ORIG_Z_MIN_PIN 92
#define ORIG_Z_MAX_PIN -1
#define ORIG_Z_ENABLE_PIN 120

#define ORIG_E0_STEP_PIN 53
#define ORIG_E0_DIR_PIN 3
#define ORIG_E0_ENABLE_PIN 124

#define ORIG_E1_STEP_PIN 122
#define ORIG_E1_DIR_PIN 121
#define ORIG_E1_ENABLE_PIN 123

#define ORIG_E2_STEP_PIN -1
#define ORIG_E2_DIR_PIN -1
#define ORIG_E2_ENABLE_PIN -1

#define FIL_SENSOR1_PIN1 93
#define FIL_SENSOR1_PIN2 24
#define FIL_JAM_SENSOR_PIN1 62
#define FIL_JAM_SENSOR_PIN2 70

#define ORIG_ZPROBE_PIN 5

// bed heater
#define HEATER_1_PIN 67
// hotend1 heater
#define HEATER_0_PIN 20
// hotend2 heater
#define HEATER_2_PIN 16

// hotend1 thermistor
#define TEMP_0_PIN 9
// bed thermistor
#define TEMP_1_PIN 14
// hotend2 thermistor
#define TEMP_2_PIN 13

// servo 
#define ORIG_SERVO_PIN 5

// hotend cooler
#define ORIG_FAN_PIN 4
#define ORIG_FAN2_PIN 25

// sdcard 
#undef SDSS
#define SDSS 55
#define SDSUPPORT 1
#define SDPOWER -1
#define ORIG_SDCARDDETECT 74
#define SDCARDDETECT ORIG_SDCARDDETECT
#define SDCARDDETECTINVERTED 0
#define ENABLE_SOFTWARE_SPI_CLASS 1
#define SD_SOFT_MISO_PIN        73
#define SD_SOFT_MOSI_PIN        43
#define SD_SOFT_SCK_PIN         42
#define EEPROM_AVAILABLE EEPROM_SDCARD


#define LED_PIN -1
#define LIGHT_PIN 17
#define BADGE_LIGHT_PIN -1
#define DOOR_PIN 64

#define ORIG_PS_ON_PIN 40
#define KILL_PIN -1
#define SUICIDE_PIN -1

#define CASE_LIGHT_PIN 17


#define CUSTOM_CONTROLLER_PINS
#define UI_DISPLAY_RS_PIN		8		// PINK.1, 88, D_RS
#define UI_DISPLAY_RW_PIN		-1
#define UI_DISPLAY_RW_PIN_NOT_USED 45 //to set state to low as it is connected
#define UI_DISPLAY_ENABLE_PIN	125		// PINK.3, 86, D_E
#define UI_DISPLAY_D0_PIN		34		// PINF.5, 92, D_D4
#define UI_DISPLAY_D1_PIN		35		// PINK.2, 87, D_D5
#define UI_DISPLAY_D2_PIN		36		// PINL.5, 40, D_D6
#define UI_DISPLAY_D3_PIN		37		// PINK.4, 85, D_D7
#define UI_DISPLAY_D4_PIN		38		// PINF.5, 92, D_D4
#define UI_DISPLAY_D5_PIN		39		// PINK.2, 87, D_D5
#define UI_DISPLAY_D6_PIN		40		// PINL.5, 40, D_D6
#define UI_DISPLAY_D7_PIN		41		// PINK.4, 85, D_D7
#define UI_DELAYPERCHAR		    320

#define UI_BACKLIGHT_PIN        78

#define BEEPER_PIN 66

#define UI_ENCODER_CLICK 76
#define UI_HAS_BACK_KEY 1
#define UI_HAS_KEYS 1
#define UI_RESET_PIN 77
#define UI_BACK_PIN 75
#define UI_PREV_PIN 44
#define UI_NEXT_PIN 10

#define E0_PINS ORIG_E0_STEP_PIN, ORIG_E0_DIR_PIN, ORIG_E0_ENABLE_PIN,
#define E1_PINS ORIG_E1_STEP_PIN, ORIG_E1_DIR_PIN, ORIG_E1_ENABLE_PIN,
#define E2_PINS ORIG_E2_STEP_PIN, ORIG_E2_DIR_PIN, ORIG_E2_ENABLE_PIN,