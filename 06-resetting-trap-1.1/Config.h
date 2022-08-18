#ifndef config_h
#define config_h

#define DAYTIME_MODE_PIN 10
#define LED_STATUS_PIN 13
#define PIR 2
//#define BEAM 7
#define ENABLE_6V_PIN 4
#define SERVO_PIN 5
//#define SERVO_DOOR_PIN 6
#define BAIT_PIN 6
#define LA_SENSE A0
#define LA_PWM 9
#define LA_FORWARD 8
#define LA_BACK 7
#define BAIT_DURATION_PIN A1
#define BAIT_DELAY_PIN A2
#define BAIT_TRIG_PIN 12

#define LAT -43.388018
#define LONG 172.525346
#define TIMEZONE 13

//#define LAT 19.513469
//#define LONG -80.566956
//#define TIMEZONE -5

#define MINUTES_AFTER_SUNSET 60
#define MINUTES_BEFORE_SUNRISE 60

#define STATUS_CODE_CAUGHT_PEST 2
#define STATUS_CODE_RTC_NOT_FOUND 3
#define STATUS_CODE_RTC_TIME_NOT_SET 4
#define STATUS_STARTING 5

#define SERVO_CLOSED_ANGLE 50   // Angle to lock the mechanism in place
#define SERVO_OPEN_ANGLE 10  // Angle to release the mechanism

#define RESET_WAIT_TIME 600000 // 10 minnutes in millisecodns 10*60*1000

#define BAIT_DURATION_MIN 2000  // 2 seconds in milliseconds 
#define BAIT_DURATION_MAX 10000  // 10 seconds in milliseconds 

#define BAIT_DELAY_MIN 600000 // 10 minutes, 10*60*1000
#define BAIT_DELAY_MAX 21600000 // 6 hours in milliseconds, 6*60*60*1000


#endif
