// Code to test the linear actuator. Running it back and forward multiple times.

#include <Servo.h>

#define SERVO_1_PIN 5
#define ENABLE_6V_PIN A0
#define SERVO_TRAP_OPEN_ANGLE 10
#define SERVO_TRAP_CLOSED_ANGLE 50
#define MOTOR_DIRECTION 4
#define MOTOR_ENABLE 3
#define MOTOR_SENSE A3
#define MOTOR_OPEN_DIRECTION LOW
#define MOTOR_CLOSE_DIRECTION HIGH

int counter = 0;

Servo trapServo;

void setup() {
  pinMode(MOTOR_DIRECTION, OUTPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);
  pinMode(MOTOR_SENSE, INPUT);
  pinMode(SERVO_1_PIN, OUTPUT);
  pinMode(ENABLE_6V_PIN, OUTPUT);

  digitalWrite(MOTOR_ENABLE, LOW);
  digitalWrite(MOTOR_DIRECTION, LOW);

  Serial.begin(57600);
  Serial.println("Start init");

  Serial.println("Resetting servos");
  trapServo.attach(SERVO_1_PIN);
  trapServo.write(SERVO_TRAP_CLOSED_ANGLE);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(2000); //Give time for servos to move to there start position
  digitalWrite(ENABLE_6V_PIN, LOW);
  Serial.println("Finished resettings servos. They should be in there initial position now.");

  Serial.println("Finished init.");
}

void loop() {
  Serial.println(counter++);
  resetTrap();
  delay(3000);
}

void resetTrap() {
  Serial.println("Opening up trap.");
  digitalWrite(MOTOR_DIRECTION, MOTOR_OPEN_DIRECTION);
  digitalWrite(MOTOR_ENABLE, HIGH);
  delay(2000);
  int noCurrentCounter = 0;
  while (true) {
    int a = analogRead(MOTOR_SENSE);
    Serial.println(a);
    if (a == 0) {
      Serial.println("no current in motors");
      noCurrentCounter++;
    } else {
      noCurrentCounter = 0;
    }
    if (noCurrentCounter > 5) {
      break;
    }
    delay(500);
  }
  digitalWrite(MOTOR_ENABLE, LOW);

  Serial.println("Moving linear actuator back");

  digitalWrite(MOTOR_DIRECTION, MOTOR_CLOSE_DIRECTION);
  digitalWrite(MOTOR_ENABLE, HIGH);
  delay(2000);
  noCurrentCounter = 0;
  while (true) {
    int a = analogRead(MOTOR_SENSE);
    Serial.println(a);
    if (a == 0) {
      Serial.println("no current in motors");
      noCurrentCounter++;
    } else {
      noCurrentCounter = 0;
    }
    if (noCurrentCounter > 5) {
      break;
    }
    delay(500);
  }
  digitalWrite(MOTOR_ENABLE, LOW);

  Serial.println("Finished resetting trap.");
}
