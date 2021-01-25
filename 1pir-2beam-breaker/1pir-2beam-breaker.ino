// Main trap has one PIR in the middle. The cage has a beam breaker sensor
// When the PIR is triggered the trap will trigger. When the beam breaker sensor is triggered the cage door will close. 
// If the beam breaker sensor is not triggered for ENTER_CAGE_MAX_TIME the trap will open again and we assume it was a false trigger.


//Pins for NAME VERSION PCB
// Note attach SENSE pin to a analog pin on the arduion

#include <RTClib.h>       // https://github.com/adafruit/RTClib
#include <Servo.h>
#include <Dusk2Dawn.h>    // https://github.com/dmkishi/Dusk2Dawn

#define DAYTIME_MODE_PIN A7
#define PIR_1 2
#define BEAM 7                  // Using PIR_2 plug
#define MOTOR_DIRECTION_PIN 4
#define MOTOR_ENABLE_PIN 3
#define MOTOR_SENSE_PIN A3          // This needs to be modded
#define MOTOR_OPEN_DIRECTION LOW
#define MOTOR_CLOSE_DIRECTION HIGH


#define TX_PIN 1
#define RX_PIN 0

#define LED_STATUS_PIN A2           //This needs to be modded

#define ENABLE_6V_PIN A0
#define SERVO_1_PIN 5 
#define SERVO_TRAP_CLOSED_ANGLE 50   // Angle to lock the mechanism in place
#define SERVO_TRAP_OPEN_ANGLE 10  // Angle to release the mechanism
#define SERVO_2_PIN 6

#define SERVO_DOOR_OPEN_ANGLE 12
#define SERVO_DOOR_CLOSED_ANGLE 98
#define SERVO_DOOR_LOCKED_ANGLE 125

#define STATUS_CODE_CAUGHT_PEST 2
#define STATUS_CODE_RTC_NOT_FOUND 3
#define STATUS_CODE_RTC_TIME_NOT_SET 4
#define STATUS_STARTING 20

#define WAIT_SECONDS_MOVE_TO_CAGE 5 * 60

// NZ, Christchurch locatoin
#define LAT -43.388018
#define LONG 172.525346

#define MINUTES_AFTER_SUNSET 60       //Number of minutes after sunset before starting trap
#define MINUTES_BEFORE_SUNRISE 60     //Number of minutes before sunrise to stop trap

Dusk2Dawn d2d_chch(LAT, LONG, 13);    // This is in UTC+13 Will be out by one hour if in UTC+12

Servo trapServo;
Servo doorServo;

RTC_DS1307 rtc;

unsigned long trapTriggerTime = 0;

enum State {
  WAITING_FOR_NIGHT,
  WAITING_FOR_MAIN_TRIGGER,
  WAITING_FOR_CAGE_TRIGGER,
  CAUGHT_SOMETHING
};

State state = WAITING_FOR_MAIN_TRIGGER;

bool trapOpen = false;
bool cageDoorOpen = false;

void setup() {
  pinMode(DAYTIME_MODE_PIN, INPUT);
  pinMode(PIR_1, INPUT);
  pinMode(BEAM, INPUT_PULLUP);
  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_SENSE_PIN, INPUT);
  pinMode(SERVO_1_PIN, OUTPUT);
  pinMode(SERVO_2_PIN, OUTPUT);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);

  //pinMode(RX_PIN, INPUT);
  //pinMode(TX_PIN, INPUT);
  //digitalWrite(RX_PIN);
  //digitalWrite(TX_PIN, LOW);

  digitalWrite(MOTOR_ENABLE_PIN, LOW);
  digitalWrite(MOTOR_DIRECTION_PIN, LOW);

  Serial.begin(57600);
  Serial.println("Start init");

  initRTC();

  Serial.println("Resetting servos");
  trapServo.attach(SERVO_1_PIN);
  trapServo.write(SERVO_TRAP_CLOSED_ANGLE);
  doorServo.attach(SERVO_2_PIN);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  doorServo.write(SERVO_DOOR_CLOSED_ANGLE);
  delay(2000);
  doorServo.write(SERVO_DOOR_OPEN_ANGLE);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(2000); //Give time for servos to move to there start position
  digitalWrite(ENABLE_6V_PIN, LOW);
  Serial.println("Finished resettings servos. They should be in there initial position now.");

  resetTrap();

  inActiveWindow(true);
  
  Serial.println("Finished init");
  blinkStatus(STATUS_STARTING, false);
  Serial.println("Waiting for PIR to trigger");
}

void loop() {

  if (!inActiveWindow(false) && state != WAITING_FOR_NIGHT && state != WAITING_FOR_CAGE_TRIGGER) {
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
      if (digitalRead(PIR_1) == HIGH) {
        Serial.println("Main PIR triggered. Closing blinds");
        trapTriggerTime = millis();
        moveServo(trapServo, SERVO_TRAP_OPEN_ANGLE, 1000);
        moveServo(trapServo, SERVO_TRAP_CLOSED_ANGLE, 1000);
        state = WAITING_FOR_CAGE_TRIGGER;
        Serial.println("Waiting for animal to move into cage");
      }
      break;

    case WAITING_FOR_CAGE_TRIGGER:
      // Waiting for the beam to trigger
      if (digitalRead(BEAM) == LOW) {
        moveServo(doorServo, SERVO_DOOR_CLOSED_ANGLE, 4000);
        moveServo(doorServo, SERVO_DOOR_LOCKED_ANGLE, 2000);
        blinkStatus(STATUS_CODE_CAUGHT_PEST, true);
      } else if (millis() - trapTriggerTime > WAIT_SECONDS_MOVE_TO_CAGE*1000) {
        Serial.println("Waited too long for animal to move into cage. Resetting trap.");
        resetTrap();
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;
  }
}


void resetTrap() {
  Serial.println("Opening up trap.");
  digitalWrite(MOTOR_DIRECTION_PIN, MOTOR_OPEN_DIRECTION);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);
  delay(2000);
  int noCurrentCounter = 0;
  while (true) {
    int a = analogRead(MOTOR_SENSE_PIN);
    if (a == 0) {
      Serial.println("no current in motors");
      noCurrentCounter++;
    } else {
      noCurrentCounter = 0;
    }
    if (noCurrentCounter > 5) {
      break;
    }
    delay(200);
  }
  digitalWrite(MOTOR_ENABLE_PIN, LOW);

  Serial.println("Moving linear actuator back");

  digitalWrite(MOTOR_DIRECTION_PIN, MOTOR_CLOSE_DIRECTION);
  digitalWrite(MOTOR_ENABLE_PIN, HIGH);
  delay(2000);
  noCurrentCounter = 0;
  while (true) {
    int a = analogRead(MOTOR_SENSE_PIN);
    if (a == 0) {
      Serial.println("no current in motors");
      noCurrentCounter++;
    } else {
      noCurrentCounter = 0;
    }
    if (noCurrentCounter > 5) {
      break;
    }
    delay(200);
  }
  digitalWrite(MOTOR_ENABLE_PIN, LOW);

  Serial.println("Finished resetting trap.");
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

void moveServo(Servo s, int angle, int mill) {
  s.write(angle);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(mill);
  digitalWrite(ENABLE_6V_PIN, LOW);
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
