// Main trap has one PIR sensor in the middle. When the PIR detects movement
// in the given time window the trap will trigger, hopefully causing the pest
// into one of the holding cages. After 10 minutes the trap will open up again
// ready to catch the next pest.


#include "rtc.h"
#include "util.h"
#include "Config.h"
#include "linearActuator.h"

//#include <RTClib.h>       // https://github.com/adafruit/RTClib
#include <Servo.h>
//#include <Dusk2Dawn.h>    // https://github.com/dmkishi/Dusk2Dawn

Servo trapServo;

LinearActuator linearActuator(LA_PWM, LA_SENSE, LA_FORWARD, LA_BACK);
RTC rtc; //TODO Rename to time Window

unsigned long lastMovementTime = 0;
//unsigned long trapResetDelayTime = 10000; //10 seconds for testing
unsigned long trapResetDelayTime = 600000; // 10 minutes
unsigned long baitDuration = 10000;  // 10 seconds
//unsigned long baitDelay = 20000; // 20 seconds for testing`
unsigned long baitDelay = 10800000; // 3 hours, 3*60*60*1000
unsigned long baitTriggerTime = 0;

enum State {
  WAITING_FOR_ACTIVE_WINDOW,
  WAITING_FOR_MAIN_TRIGGER,
  WAITING_FOR_RESET
};

State state = WAITING_FOR_MAIN_TRIGGER;

void setup() {
  //======= SETUP IO PINS ================
  linearActuator.setup();
  rtc.setup();
  pinMode(PIR, INPUT);
  pinMode(SERVO_PIN, OUTPUT);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  pinMode(LED_STATUS_PIN, OUTPUT);
  pinMode(BAIT_PIN, OUTPUT);
  digitalWrite(BAIT_PIN, LOW);


  //============ INIT SYSTEMS ===============
  Serial.begin(57600);
  Serial.println("Start init systems");
  rtc.init();
  linearActuator.init();
  initServo();
  
  resetLinearActuator();
  
  triggerBait();
  rtc.isInActiveWindow(true); // Print out time status

  Serial.println(F("Finished init"));
  blinkStatus(STATUS_STARTING, false);
  setState(WAITING_FOR_MAIN_TRIGGER);
  Serial.println(F("Waiting for PIR to trigger"));
}

void setState(State newState) {
  if (newState == state) {
    return;
  }
  state = newState;
  switch (state) {
    case WAITING_FOR_ACTIVE_WINDOW:
      Serial.println(F("Switching state to: Waiting for active window"));
      break;
    case WAITING_FOR_MAIN_TRIGGER:
      Serial.println(F("Switching state to: Waiting for main trigger"));
      break;
    case WAITING_FOR_RESET:
      Serial.println(F("Switching state to: Waiting for trap reset"));
      break;
    default:
      Serial.println(F("Switching to unknown state"));
      break;
  }
}

void loop() {
  switch (state) {
    case WAITING_FOR_ACTIVE_WINDOW:
      if (rtc.isInActiveWindow(false)) {
        rtc.isInActiveWindow(true);
        Serial.println("Active window started. Waiting for main trigger.");
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;

    case WAITING_FOR_MAIN_TRIGGER:
      if (!rtc.isInActiveWindow(false)) {
        rtc.isInActiveWindow(true);
        Serial.println("No longer in active window. Waiting for active window.");
        state = WAITING_FOR_ACTIVE_WINDOW;
        break;
      }
      //Waiting for the PIR to be triggered
      if (digitalRead(PIR) == LOW) {
        Serial.println("Main PIR triggered. Closing blinds");
        lastMovementTime = millis();

        moveServo(trapServo, SERVO_OPEN_ANGLE, 1000);
        moveServo(trapServo, SERVO_CLOSED_ANGLE, 1000);
        state = WAITING_FOR_RESET;
        Serial.println("Waiting for animal to move into cage");
      }
      // Check if food should be dispensed
      if (millis() - baitTriggerTime > baitDelay) {
        triggerBait();
      }
      break;

    case WAITING_FOR_RESET:
      if (digitalRead(PIR) == LOW) {
        Serial.println("Movement detected, reseting wait time.");
        lastMovementTime = millis();
      }
      if (millis() - lastMovementTime > RESET_WAIT_TIME) {
        Serial.println("Waited long enough for the pest to move into a cage. Resetting trap.");
        resetLinearActuator();
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;
  }
}

void resetLinearActuator() {
  linearActuator.back();
  linearActuator.forward();
  moveServo(trapServo, SERVO_OPEN_ANGLE, 1000);
  moveServo(trapServo, SERVO_CLOSED_ANGLE, 1000);
  linearActuator.back();
}

//===========SERVOS===============
void initServo() {
  Serial.print("Running Servo init...  ");
  trapServo.attach(SERVO_PIN);
  moveServo(trapServo, SERVO_CLOSED_ANGLE, 1000);
  Serial.println("Done.");
}

void moveServo(Servo s, int angle, int mill) {
  s.write(angle);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(mill);
  digitalWrite(ENABLE_6V_PIN, LOW);
}

//===================BAIT==============
void initBait() {
  Serial.print("Running Bait init...  ");
  triggerBait();
  Serial.println("Done");
}

void triggerBait() {
  Serial.println("running bait");
  digitalWrite(BAIT_PIN, HIGH);
  delay(baitDuration);
  digitalWrite(BAIT_PIN, LOW);
  baitTriggerTime = millis();
  delay(10000); // Wait 10 seconds so if the bait movement triggered the PIR it has time to turn off.
}
