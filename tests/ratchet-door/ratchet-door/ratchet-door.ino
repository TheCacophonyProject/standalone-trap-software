#define ENABLE_6V_PIN 4
#define SERVO_PIN 10
#define SERVO_DOWN 1700
#define SERVO_LOCK 1200
#define SERVO_DOOR_HOLD 1550
#define SERVO_RELEASE 1050
#define SERVO_TRIG_HOLD 1300
#define RATCHET_CYCLES 8
#include <Servo.h>

Servo s;

void setup() {
  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, LOW);
  Serial.begin(57600);
  Serial.println("Starting ratchet-door test");
  s.attach(SERVO_PIN, 1000, 2500);
  digitalWrite(ENABLE_6V_PIN, HIGH);
}

void loop() {
  resetRatchetDoor();
  triggerRatchetDoor();

}

void resetRatchetDoor() {
  digitalWrite(ENABLE_6V_PIN, HIGH);
  Serial.println("Starting ratchet cycle");
  for (int i = 0; i < RATCHET_CYCLES; i++) {
    Serial.print("Cycle: ");
    Serial.println(i+1);
    s.writeMicroseconds(SERVO_DOWN);
    delay(1000);
    s.writeMicroseconds(SERVO_LOCK);
    delay(1000);
  }
  s.writeMicroseconds(SERVO_TRIG_HOLD);
  digitalWrite(ENABLE_6V_PIN, LOW);
}

void triggerRatchetDoor() {
  Serial.println("Releasing door");
  digitalWrite(ENABLE_6V_PIN, HIGH);
  s.writeMicroseconds(SERVO_RELEASE);
  delay(1000);
  digitalWrite(ENABLE_6V_PIN, LOW);
}
