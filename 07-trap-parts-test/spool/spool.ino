#include <Servo.h> 

Servo s;
#define SERVO_PIN 5    // Servo 1
//#define SERVO_PIN 11 // Servo 2
#define ENABLE_6V_PIN 4


//===============================
#define SERVO_HOME 2300     // Home position, waiting to trigger.
#define SERVO_TRIGGER 2450  // Angle to trigger spool.
#define SERVO_RESET 550     // Angle to reset spool.
#define WAIT_MILLS 10000    // Time in ms to wait between cycles.
//===============================

void setup() { 
  pinMode(ENABLE_6V_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  s.attach(SERVO_PIN, 500, 2500);
  s.writeMicroseconds(SERVO_HOME);
  delay(1000);
  Serial.begin(57600);
  Serial.println("Finished init");
}

void loop() {
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
