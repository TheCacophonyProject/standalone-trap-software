#include "rtc.h"
#include "Config.h"
#include "util.h"

#include <RTClib.h>
#include <Dusk2Dawn.h>

Dusk2Dawn d2d_chch(LAT, LONG, 13);

#define FORCE_SET false

void RTC::setup() {
  pinMode(DAYTIME_MODE_PIN, INPUT_PULLUP);
}

void RTC::init() {
  Serial.print("Running RTC init...  ");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    return STATUS_CODE_RTC_NOT_FOUND;
  } else if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running.");
    return STATUS_CODE_RTC_TIME_NOT_SET;
  }
  Serial.println("Done");
}

void RTC::setTime() {
  if (!rtc.begin()) {
    blinkStatus(STATUS_CODE_RTC_NOT_FOUND, false);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, setting the time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else if (FORCE_SET_RTC) {
    Serial.println("Forcing writing of RTC time.");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else {
    Serial.println("RTC is already running. Not writing new time to RTC.");
  }
}

DateTime RTC::getDateTime() {
    return rtc.now();
}

bool RTC::isInActiveWindow(bool printMessages) {

  if (analogRead(DAYTIME_MODE_PIN) == 0) {
    if (printMessages) {
       Serial.println("24/7 switch is on");
    }
    return true;
  }
  
  DateTime now = rtc.now();
  int minutesFromMidnight = now.hour()*60 + now.minute();
  int startMinute = d2d_chch.sunset(now.year(), now.month(), now.day(), false) + MINUTES_AFTER_SUNSET;
  int stopMinute = d2d_chch.sunrise(now.year(), now.month(), now.day(), false) - MINUTES_BEFORE_SUNRISE;
  
  if (printMessages) {  
    Serial.print("Time of day: ");
    printMIn24(minutesFromMidnight);
    Serial.print("Startig at : ");
    printMIn24(startMinute);
    Serial.print("Stopping at :");
    printMIn24(stopMinute);
  }

  if (minutesFromMidnight < stopMinute) {
    if (printMessages) {
      Serial.print("Before Sunrise, will stop in: ");
      int minutesLeft = stopMinute - minutesFromMidnight;
      printMIn24(minutesLeft);
    }
    return true;
  }
  else if (minutesFromMidnight > startMinute) {
    if (printMessages) {
      Serial.print("After Sunset, will stop in: ");
      int minutesLeft = stopMinute + 24*60 - minutesFromMidnight;
      printMIn24(minutesLeft);
    }
    return true;
  }
  else {
    if (printMessages) {
      Serial.print("During off period. Need to wait: ");
      printMIn24(startMinute - minutesFromMidnight);
    }
    return false;
  }
}
