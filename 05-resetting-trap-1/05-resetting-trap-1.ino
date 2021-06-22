// Main trap has one PIR sensor in the middle. When the PIR detects movement
// in the given time window the trap will trigger, hopefully causing the pest
// into one of the holding cages. After 10 minutes the trap will open up again
// ready to catch the next pest.


#include "rtc.h"
#include "util.h"
#include "Config.h"
#include "linearActuator.h"
#include <Servo.h>

LinearActuator linearActuator(LA_PWM, LA_SENSE, LA_FORWARD, LA_BACK);
RTC rtc; //TODO Rename to time Window

unsigned long lastMovementTime = 0;
unsigned long baitTriggerTime = 0;
Servo trapServo;

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
  pinMode(6, INPUT);
  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, LOW);
  pinMode(LED_STATUS_PIN, OUTPUT);
  digitalWrite(LED_STATUS_PIN, LOW);
  pinMode(BAIT_PIN, OUTPUT);
  digitalWrite(BAIT_PIN, LOW);
  pinMode(BAIT_TRIG_PIN, INPUT_PULLUP);
  pinMode(BAIT_DURATION_PIN, INPUT);
  pinMode(BAIT_DELAY_PIN, INPUT);

  //============ INIT SYSTEMS ===============
  Serial.begin(57600);
  Serial.println("Start init systems");
  rtc.init();
  rtc.setTime();
  rtc.isInActiveWindow(true); // Print out time status
  linearActuator.init();
  initServo();
  resetTrap();
  triggerBait();
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
      checkBait(false);
      break;

    case WAITING_FOR_RESET:
      if (digitalRead(PIR) == LOW) {
        Serial.println("Movement detected, reseting wait time.");
        lastMovementTime = millis();
      }
      if (millis() - lastMovementTime > RESET_WAIT_TIME) {
        Serial.println("Waited long enough for the pest to move into a cage. Resetting trap.");
        resetTrap();
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;
  }
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

void checkBait(bool printMessages) {
  if (digitalRead(BAIT_TRIG_PIN) == LOW) {
    triggerBait();
  }
  long d = analogRead(BAIT_DELAY_PIN);
  if (d > 1000) {
    if (printMessages) {
      Serial.println("bait disabled");
    }
    return;
  }
  d = map(d, 0, 1024, BAIT_DELAY_MIN, BAIT_DELAY_MAX);
  if (printMessages) {
    Serial.print("bait delay: ");
    Serial.println(d);
  }
  if (millis() - baitTriggerTime > d) {
    triggerBait();
  }
}

void triggerBait() {
  long duration = map(analogRead(BAIT_DURATION_PIN), 0, 1024, BAIT_DURATION_MIN, BAIT_DURATION_MAX);
  Serial.print("running bait for: ");
  Serial.print(duration);
  digitalWrite(BAIT_PIN, HIGH);
  delay(duration);
  digitalWrite(BAIT_PIN, LOW);
  baitTriggerTime = millis();
  Serial.print("Stopping bait and waiting 10 seconds before returning to main loop");
  delay(10000); // Wait 10 seconds so if the bait movement triggered the PIR it has time to turn off.
}


//===================================
void resetTrap() {
  linearActuator.back();
  linearActuator.forward();
  moveServo(trapServo, SERVO_OPEN_ANGLE, 1000);
  moveServo(trapServo, SERVO_CLOSED_ANGLE, 1000);
  linearActuator.back();
}
