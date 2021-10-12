#include <Servo.h> 

Servo s1;
Servo s2;
#define SERVO_1_PIN 5
#define SERVO_2_PIN 11
#define SERVO_HOME 2300
#define SERVO_TRIGGER 2450
#define SERVO_RESET 550
//#define WAIT_MILLS 600000
#define WAIT_MILLS 180000
#define ENABLE_6V_PIN 4

void setup() { 
  pinMode(SERVO_1_PIN, OUTPUT);
  pinMode(SERVO_2_PIN, OUTPUT);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  s1.attach(SERVO_1_PIN, 500, 2500);
  s1.writeMicroseconds(SERVO_HOME);
  delay(1000);
  s2.attach(SERVO_2_PIN, 500, 2500);
  s2.writeMicroseconds(SERVO_HOME);
  delay(1000);
  Serial.begin(57600);
  Serial.println("Finished init");
  
  Serial.println("Running S2");
  cycle(s2);
  Serial.println("Running S1");
  cycle(s1);
  
}

void loop() {
  cycle(s2);
  delay(WAIT_MILLS);
}


void cycle(Servo s) {
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(1000);
  s.writeMicroseconds(SERVO_RESET);
  delay(3000);
  s.writeMicroseconds(SERVO_HOME);
  delay(3000);
  s.writeMicroseconds(SERVO_TRIGGER);
  delay(3000);
  s.writeMicroseconds(SERVO_HOME);
  delay(3000);
  //digitalWrite(ENABLE_6V_PIN, LOW);
}
