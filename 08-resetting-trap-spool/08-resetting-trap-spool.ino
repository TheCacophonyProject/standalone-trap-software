// Main trap has one PIR sensor in the middle. When the PIR detects movement
// in the given time window the trap will trigger, hopefully causing the pest
// into one of the holding cages. After 10 minutes the trap will open up again
// ready to catch the next pest.

#include "rtc.h"
#include "util.h"
#include "Config.h"
#include "linearActuator.h"
#include <Servo.h>

//LinearActuator linearActuator(LA_PWM, LA_SENSE, LA_FORWARD, LA_BACK);
RTC rtc; //TODO Rename to time Window

unsigned long lastMovementTime = 0;
unsigned long baitTriggerTime = 0;
Servo spoolServo1;
Servo spoolServo2;

enum State {
  WAITING_FOR_ACTIVE_WINDOW,
  WAITING_FOR_MAIN_TRIGGER,
  WAITING_FOR_RESET
};

State state = WAITING_FOR_MAIN_TRIGGER;

void setup() {
  //======= SETUP IO PINS ================
//  linearActuator.setup();
  rtc.setup();
  pinMode(PIR, INPUT);
  pinMode(6, INPUT);
  pinMode(9, INPUT);
  pinMode(SERVO_1_PIN, OUTPUT);
  pinMode(SERVO_2_PIN, OUTPUT);
  digitalWrite(SERVO_1_PIN, LOW);
  digitalWrite(SERVO_2_PIN, LOW);
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
  rtc.isInActiveWindow(true); // Print out time status
//  linearActuator.init();
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
        Serial.println("manual trigger of bait");
        triggerBait();
      }
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

        triggerTrap();
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


//===================BAIT==============
void initBait() {
  Serial.print("Running Bait init...  ");
  triggerBait();
  Serial.println("Done");
}

void checkBait(bool printMessages) {
  if (digitalRead(BAIT_TRIG_PIN) == LOW) {
    Serial.println("manual trigger of bait");
    triggerBait();
  }
  long d = analogRead(BAIT_DELAY_PIN);
  if (d > 1000) {
    if (printMessages) {
      Serial.println("bait disabled");
    }
    return;
  }
  d = map(d, 0, 1024, BAIT_DELAY_MIN/1000, BAIT_DELAY_MAX/1000); // was getting overflow 
  d = d*1000;
  if (printMessages) {
    Serial.print("bait delay(ms): ");
    Serial.println(d);
  }
  if (millis() - baitTriggerTime > d) {
    triggerBait();
  }
}

void triggerBait() {
  long duration = map(analogRead(BAIT_DURATION_PIN), 0, 1024, BAIT_DURATION_MIN, BAIT_DURATION_MAX);
  Serial.print("running bait for: ");
  Serial.println(duration);
  digitalWrite(BAIT_PIN, HIGH);
  delay(duration);
  digitalWrite(BAIT_PIN, LOW);
  baitTriggerTime = millis();
  Serial.println("Stopping bait and waiting 10 seconds before returning to main loop");
  delay(10000); // Wait 10 seconds so if the bait movement triggered the PIR it has time to turn off.
}

//===========SERVOS===============
void initServo() {
  Serial.print("Running Servo init...  ");
  spoolServo1.attach(SERVO_1_PIN);
  moveServo(spoolServo1, SERVO_HOME, 2000);
  spoolServo2.attach(SERVO_2_PIN);
  moveServo(spoolServo2, SERVO_HOME, 2000);
  Serial.println("Done.");
}

void moveServo(Servo s, int sig_us, int mill) {
  s.writeMicroseconds(sig_us);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(mill);
  digitalWrite(ENABLE_6V_PIN, LOW);
}


void resetSpool(Servo s) {
  Serial.println("Resetting a spool");
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(200);
  s.writeMicroseconds(SERVO_RESET);
  delay(3000);
  s.writeMicroseconds(SERVO_HOME);
  delay(2000);
  Serial.println("Done resetting a spool.");
  digitalWrite(ENABLE_6V_PIN, LOW);
}

void resetTrap() {
  Serial.println("Resetting trap");
  resetSpool(spoolServo1);
  resetSpool(spoolServo2);
  Serial.println("Finished resetting trap");
}

void triggerTrap() {
  Serial.println("Trigger spools");
  digitalWrite(ENABLE_6V_PIN, HIGH);
  spoolServo1.writeMicroseconds(SERVO_TRIGGER);
  spoolServo2.writeMicroseconds(SERVO_TRIGGER);
  delay(2000);
  spoolServo1.writeMicroseconds(SERVO_HOME);
  delay(1000);
  spoolServo2.writeMicroseconds(SERVO_HOME);
  delay(1000);
  digitalWrite(ENABLE_6V_PIN, LOW);
  Serial.println("Done");
}


/*
void foo() {
  digitalWrite(ENABLE_6V_PIN, HIGH);
  Serial.println("Resetting spool");
  delay(1000);
  s.writeMicroseconds(SERVO_RESET);
  delay(3000);
  Serial.println("Moving to HOME to wait");
  s.writeMicroseconds(SERVO_HOME);
  delay(3000);
  digitalWrite(ENABLE_6V_PIN, LOW);
  Serial.println("Waiting at home with servo powered off");
  delay(3000);
  Serial.println("Trigger");
  digitalWrite(ENABLE_6V_PIN, HIGH);
  s.writeMicroseconds(SERVO_TRIGGER);
  delay(3000);
  s.writeMicroseconds(SERVO_HOME);
  delay(3000);
  digitalWrite(ENABLE_6V_PIN, LOW);
  delay(WAIT_MILLS);
}
*/







//========== TEST ==================

void testRTC() {
  //rtc.init();
  //while(true) {
  //  rtc.isInActiveWindow(true); // Print out time status
   // delay(5000);
 // }
}
/*
void servoAngleTest() {
  trapServo.attach(SERVO_PIN, 500, 2500);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  Serial.println("Waiting for input");
  while (true) {
    String s = Serial.readStringUntil('\n');
    digitalWrite(ENABLE_6V_PIN, HIGH);
    if (s != "") {
      Serial.println(s);
      int i = s.toInt();
      Serial.println(i);
      trapServo.writeMicroseconds(i);
      Serial.println("Waiting for input");
    }
  }
}
*/

void testFeeder() {
  while(true){
    Serial.println("feeder on");
    digitalWrite(BAIT_PIN, HIGH);
    delay(5000);

    Serial.println("feeder off");
    digitalWrite(BAIT_PIN, LOW);
    delay(5000);
  }
  
}

void testDigitalIO() {
  while(true) {
    Serial.println("====================================");
    Serial.print("bait trig: ");
    Serial.println(digitalRead(BAIT_TRIG_PIN));
    
    Serial.print("bait duration: ");
    Serial.println(analogRead(BAIT_DURATION_PIN));
  
    Serial.print("bait delay: ");
    Serial.println(analogRead(BAIT_DELAY_PIN));

    Serial.print("PIR: ");
    Serial.println(digitalRead(PIR));

    Serial.print("24/7: ");
    Serial.println(digitalRead(DAYTIME_MODE_PIN));
    delay(1000);
  }
}
