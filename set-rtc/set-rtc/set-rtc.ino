// This gets run on the Arduino to write the time to the RTC.
// The time is written to the time that the code was compiled.
// After installing and running this code you should install something else because when the 
// Arduion resets if this is still installed the time will be written again to the time that
// this was compiled at, likely not being the right time anymore..

#include <RTClib.h>   // https://github.com/adafruit/RTClib
#include <Dusk2Dawn.h>    // https://github.com/dmkishi/Dusk2Dawn

RTC_DS1307 rtc;

#define FORCE_SET false

#define DAYTIME_MODE_PIN A7

// NZ, Christchurch locatoin
#define LAT -43.388018
#define LONG 172.525346

#define MINUTES_AFTER_SUNSET 150       //Number of minutes after sunset before starting trap
#define MINUTES_BEFORE_SUNRISE 60     //Number of minutes before sunrise to stop trap

Dusk2Dawn d2d_chch(LAT, LONG, 13);

void setup() {
  pinMode(DAYTIME_MODE_PIN, INPUT);
  Serial.begin(57600);
  Serial.println("Writing time to RTC.");
  
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }
  Serial.println("Found RTC");

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  } else if (FORCE_SET) {
    Serial.println("RTC is running but am forcing to write a new time");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  } else {
    Serial.println("RTC is already running. Not writing time to RTC");
  }
  
}

void setTime() {
  // When time needs to be set on a new device, or after a power loss, the
  // following line sets the RTC to the date & time this sketch was compiled  
  Serial.println("Setting date on RTC");
  Serial.println(__DATE__);
  Serial.println(__TIME__);
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
}

void loop() {
  printDateTime(rtc.now());
  isInActiveWindow();
  delay(5000);
}

void printDateTime(DateTime now) {
  Serial.print(now.year());
  Serial.print("/");
  Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());
}

bool isInActiveWindow() {

  if (analogRead(DAYTIME_MODE_PIN) == 0) {
    Serial.println("24/7 switch is on");
    return true;
  }
  
  DateTime now = rtc.now();
  int minutesFromMidnight = now.hour()*60 + now.minute();
  int startMinute = d2d_chch.sunset(now.year(), now.month(), now.day(), false) + MINUTES_AFTER_SUNSET;
  int stopMinute = d2d_chch.sunrise(now.year(), now.month(), now.day(), false) - MINUTES_BEFORE_SUNRISE;

  Serial.print("Time of day: ");
  printMIn24(minutesFromMidnight);
  Serial.print("Startig at : ");
  printMIn24(startMinute);
  Serial.print("Stopping at :");
  printMIn24(stopMinute);

  if (minutesFromMidnight < stopMinute) {
    Serial.print("Before Sunrise, will stop in: ");
    int minutesLeft = stopMinute - minutesFromMidnight;
    printMIn24(minutesLeft);
    return true;
  }
  else if (minutesFromMidnight > startMinute) {
    Serial.print("After Sunset, will stop in: ");
    int minutesLeft = stopMinute + 24*60 - minutesFromMidnight;
    printMIn24(minutesLeft);
    return true;
  }
  else {
    Serial.print("During off period. Need to wait: ");
    printMIn24(startMinute - minutesFromMidnight);
    return false;
  }
}

void printMIn24(int m) {
  int h = m/60;
  m = m - 60*h;
  if (h<10) {
    Serial.print("0");
  }
  Serial.print(h);
  Serial.print(":");
  if (m<10) {
    Serial.print("0");
  }
  Serial.println(m);
}
