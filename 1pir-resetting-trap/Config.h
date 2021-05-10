#ifndef config_h
#define config_h

#define DAYTIME_MODE_PIN 1
#define LED_STATUS_PIN 1
#define PIR 2
#define ENABLE_6V_PIN A0
#define SERVO_PIN 5 
#define BAIT_PIN A1
#define LA_SENSE 4
#define LA_PWM 3
#define LA_FORWARD A3
#define LA_BACK 1


#define LAT -43.388018
#define LONG 172.525346

#define MINUTES_AFTER_SUNSET 60
#define MINUTES_BEFORE_SUNRISE 60

#define STATUS_CODE_RTC_NOT_FOUND 3
#define STATUS_CODE_RTC_TIME_NOT_SET 4
#define STATUS_STARTING 20

#define FORCE_SET_RTC false

#define SERVO_CLOSED_ANGLE 50   // Angle to lock the mechanism in place
#define SERVO_OPEN_ANGLE 10  // Angle to release the mechanism

#define RESET_WAIT_TIME 600000 // 10*60*1000

#endif
