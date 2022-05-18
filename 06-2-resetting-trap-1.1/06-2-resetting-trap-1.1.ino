// Main trap has one PIR sensor in the middle. When the PIR detects movement
// in the given time window the trap will trigger, hopefully causing the pest
// into one of the holding cages. After 10 minutes the trap will open up again
// ready to catch the next pest.

#include "rtc.h"
#include "util.h"
#include "Config.h"
#include "linearActuator.h"
#include <Servo.h>
#include <EEPROM.h>

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
  pinMode(9, INPUT);
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
  Serial.println(F("Start init systems"));
  Serial.println(F("06-2-resettings-trap-1.1 Trigger once a night."));


  //============ TESTS ======================
  //testRTC();
  //linearActuator.init();
  //testRampPWMOnAndOff();
  //testPWM();
  //testLinearActuator();
  //testFeeder();
  //testDigitalIO();
  //servoAngleTest();

  rtc.init(DateTime(F(__DATE__), F(__TIME__)));
  EEPROM.write(100, 0);
  rtc.isInActiveWindow(true); // Print out time status
  linearActuator.init();
  initServo();
  resetTrap();
  checkBait(true);
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
      if (digitalRead(BAIT_TRIG_PIN) == LOW) {
        Serial.println(F("manual trigger of bait"));
        triggerBait();
      }
      if (rtc.isInActiveWindow(false)) {
        rtc.isInActiveWindow(true);
        Serial.println(F("Active window started. Waiting for main trigger."));
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;

    case WAITING_FOR_MAIN_TRIGGER:
      if (rtc.nightOfTheWeek() == EEPROM.read(100)) {
        Serial.print(F("Triggered on: "));
        Serial.println(EEPROM.read(100));
        Serial.print(F("Night of the week: "));
        Serial.println(rtc.nightOfTheWeek());
        Serial.println(F("Have triggered tonight already"));
        delay(5000);
        break;
      }

      if (!rtc.isInActiveWindow(false)) {
        rtc.isInActiveWindow(true);
        Serial.println(F("No longer in active window. Waiting for active window."));
        state = WAITING_FOR_ACTIVE_WINDOW;
        break;
      }
      //Waiting for the PIR to be triggered
      if (digitalRead(PIR) == LOW) {
        Serial.println(F("Main PIR triggered. Closing blinds in 2 seconds"));
        lastMovementTime = millis();
        delay(2000);        
        moveServo(trapServo, SERVO_OPEN_ANGLE, 1000);
        moveServo(trapServo, SERVO_CLOSED_ANGLE, 1000);
        state = WAITING_FOR_RESET;
        Serial.println(F("Waiting for animal to move into cage"));

        EEPROM.write(100, rtc.nightOfTheWeek());
      }
      // Check if food should be dispensed
      checkBait(false);
      break;

    case WAITING_FOR_RESET:
      if (digitalRead(PIR) == LOW) {
        Serial.println(F("Movement detected, reseting wait time."));
        lastMovementTime = millis();
      }
      if (millis() - lastMovementTime > RESET_WAIT_TIME) {
        Serial.println(F("Waited long enough for the pest to move into a cage. Resetting trap."));
        resetTrap();
        state = WAITING_FOR_MAIN_TRIGGER;
      }
      break;
  }
}

//===========SERVOS===============
void initServo() {
  Serial.print(F("Running Servo init...  "));
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
  Serial.print(F("Running Bait init...  "));
  triggerBait();
  Serial.println(F("Done"));
}

void checkBait(bool printMessages) {
  if (digitalRead(BAIT_TRIG_PIN) == LOW) {
    Serial.println(F("manual trigger of bait"));
    triggerBait();
  }
  long d = analogRead(BAIT_DELAY_PIN);
  if (d > 1000) {
    if (printMessages) {
      Serial.println(F("bait disabled"));
    }
    return;
  }
  d = map(d, 0, 1024, BAIT_DELAY_MIN/1000, BAIT_DELAY_MAX/1000); // was getting overflow 
  d = d*1000;
  if (printMessages) {
    Serial.print(F("bait delay(ms): "));
    Serial.println(d);
  }
  if (millis() - baitTriggerTime > d) {
    triggerBait();
  }
}

void triggerBait() {
  long duration = map(analogRead(BAIT_DURATION_PIN), 0, 1024, BAIT_DURATION_MIN, BAIT_DURATION_MAX);
  Serial.print(F("running bait for: "));
  Serial.println(duration);
  digitalWrite(BAIT_PIN, HIGH);
  delay(duration);
  digitalWrite(BAIT_PIN, LOW);
  baitTriggerTime = millis();
  Serial.println(F("Stopping bait and waiting 10 seconds before returning to main loop"));
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

//========== TEST ==================

void testRTC() {
  rtc.init(DateTime(F(__DATE__), F(__TIME__)));
  while(true) {
    rtc.isInActiveWindow(true); // Print out time status
    delay(5000);
  }
}

void servoAngleTest() {
  trapServo.attach(SERVO_PIN, 500, 2500);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  Serial.println(F("Waiting for input"));
  while (true) {
    String s = Serial.readStringUntil('\n');
    digitalWrite(ENABLE_6V_PIN, HIGH);
    if (s != "") {
      Serial.println(s);
      int i = s.toInt();
      Serial.println(i);
      trapServo.writeMicroseconds(i);
      Serial.println(F("Waiting for input"));
    }
  }
}

void testFeeder() {
  while(true){
    Serial.println(F("feeder on"));
    digitalWrite(BAIT_PIN, HIGH);
    delay(5000);

    Serial.println(F("feeder off"));
    digitalWrite(BAIT_PIN, LOW);
    delay(5000);
  }
  
}

void testDigitalIO() {
  while(true) {
    Serial.println(F("===================================="));
    Serial.print(F("bait trig: "));
    Serial.println(digitalRead(BAIT_TRIG_PIN));
    
    Serial.print(F("bait duration: "));
    Serial.println(analogRead(BAIT_DURATION_PIN));
  
    Serial.print(F("bait delay: "));
    Serial.println(analogRead(BAIT_DELAY_PIN));

    Serial.print(F("PIR: "));
    Serial.println(digitalRead(PIR));

    Serial.print(F("24/7: "));
    Serial.println(digitalRead(DAYTIME_MODE_PIN));
    delay(1000);
  }
}
