#ifndef config_h
#define config_h

#define DAYTIME_MODE_PIN 3
#define LED_STATUS_PIN 13
#define PIR 2
//#define BEAM 7
#define ENABLE_6V_PIN 4
#define SERVO_PIN 5
//#define SERVO_DOOR_PIN 6
#define BAIT_PIN 10
#define LA_SENSE A0
#define LA_PWM 9
#define LA_FORWARD 7
#define LA_BACK 8


#define LAT -43.388018
#define LONG 172.525346

#define MINUTES_AFTER_SUNSET 60
#define MINUTES_BEFORE_SUNRISE 60

#define STATUS_CODE_CAUGHT_PEST 2
#define STATUS_CODE_RTC_NOT_FOUND 3
#define STATUS_CODE_RTC_TIME_NOT_SET 4
#define STATUS_STARTING 20

#define FORCE_SET_RTC false

#define SERVO_CLOSED_ANGLE 50   // Angle to lock the mechanism in place
#define SERVO_OPEN_ANGLE 10  // Angle to release the mechanism

#define RESET_WAIT_TIME 600000 // 10 minnutes in millisecodns 10*60*1000

#define BAIT_DURATION 10000  // 10 seconds in milliseconds 

#define BAIT_DELAY 10800000 // 3 hours in milliseconds, 3*60*60*1000

#endif
