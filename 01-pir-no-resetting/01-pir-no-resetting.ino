// Main trap has one PIR in the middle and when motion is detected it triggers the trap to close.

#include <RTClib.h>       // https://github.com/adafruit/RTClib
#include <Servo.h>
#include <Dusk2Dawn.h>    // https://github.com/dmkishi/Dusk2Dawn

//========== PINS ==================
#define DAYTIME_MODE_PIN A7
#define PIR_PIN 2
#define LED_STATUS_PIN A2
#define ENABLE_6V_PIN A0
#define SERVO_PIN 5

//========== Status codes ===========
#define STATUS_CODE_CAUGHT_PEST 2
#define STATUS_CODE_RTC_NOT_FOUND 3
#define STATUS_CODE_RTC_TIME_NOT_SET 4
#define STATUS_STARTING 20


#define SERVO_TRAP_CLOSED_ANGLE 50
#define SERVO_TRAP_OPEN_ANGLE 10
// NZ, Christchurch locatoin
#define LAT -43.388018
#define LONG 172.525346
#define MINUTES_AFTER_SUNSET 60       //Number of minutes after sunset before starting trap
#define MINUTES_BEFORE_SUNRISE 60     //Number of minutes before sunrise to stop trap

Dusk2Dawn d2d_chch(LAT, LONG, 13);
Servo trapServo;
RTC_DS1307 rtc;

enum State {
  WAITING_FOR_NIGHT,
  WAITING_FOR_MAIN_TRIGGER
};

State state = WAITING_FOR_MAIN_TRIGGER;

void setup() {
  pinMode(DAYTIME_MODE_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, LOW);

  Serial.begin(57600);
  Serial.println("Start init");
  Serial.println("Resetting servo");
  trapServo.attach(SERVO_PIN);
  moveServo(trapServo, SERVO_TRAP_CLOSED_ANGLE, 1000);
  Serial.println("Finished resettings servos. They should be in there initial position now.");
  initRTC();
  inActiveWindow(true);
  Serial.println("Finished init");
  blinkStatus(STATUS_STARTING, false);
  Serial.println("Waiting for PIR to trigger");
}

void loop() {

  if (!inActiveWindow(false) && state != WAITING_FOR_NIGHT) {
    inActiveWindow(true);
    Serial.println("No longer in active window. Waiting for active window.");
    state = WAITING_FOR_NIGHT;
  }

  switch (state) {
    case WAITING_FOR_NIGHT:
      //Wait until it is night then 
      if (inActiveWindow(false)) {
        inActiveWindow(true);
        Serial.println("Active window started. Waiting for main trigger.");
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;

    case WAITING_FOR_MAIN_TRIGGER:
      //Waiting for the PIR to be triggered
      if (digitalRead(PIR_PIN) == HIGH) {
        Serial.println("Main PIR triggered. Closing blinds");
        moveServo(trapServo, SERVO_TRAP_OPEN_ANGLE, 1000);
        moveServo(trapServo, SERVO_TRAP_CLOSED_ANGLE, 1000);
        blinkStatus(STATUS_CODE_CAUGHT_PEST, true);
      }
      break;
  }
}

void blinkStatus(int code, bool loopForever) {
  switch (code) {
    case STATUS_CODE_CAUGHT_PEST:
      Serial.println("Pest caught in trap");
      break;
    case STATUS_CODE_RTC_NOT_FOUND:
      Serial.println("RTC not found");
      break;
    case STATUS_CODE_RTC_TIME_NOT_SET:
      Serial.println("RTC Time not set");
      break;
    case STATUS_STARTING:
      Serial.println("Starting LED blink");
      break;
    default: 
      Serial.print("Unknown status code: ");
      Serial.println(code);
      break;
  }
  for (int x = 0; x < code; x++) {
    digitalWrite(LED_STATUS_PIN, HIGH);
    delay(500);
    digitalWrite(LED_STATUS_PIN, LOW);
    delay(500);
  }
  if (loopForever) {
    delay(2000);
    blinkStatus(code, loopForever);
  }
}

void initRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    blinkStatus(STATUS_CODE_RTC_NOT_FOUND, true);
  } else if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running.");
    blinkStatus(STATUS_CODE_RTC_TIME_NOT_SET, true);
  }
  Serial.println("Found RTC");
  printDateTime(rtc.now());
}

bool inActiveWindow(bool printMessages) {

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

bool daytimeMode() {
  Serial.println(analogRead(DAYTIME_MODE_PIN));
  return (analogRead(DAYTIME_MODE_PIN) == 0);
}


void moveServo(Servo s, int angle, int mill) {
  s.write(angle);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(mill);
  digitalWrite(ENABLE_6V_PIN, LOW);
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
