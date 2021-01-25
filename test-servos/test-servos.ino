#define SERVO_1_PIN 5
#define SERVO_2_PIN 6
#define ENABLE_6V_PIN A0

#define SERVO_TRAP_OPEN_ANGLE 10
#define SERVO_TRAP_CLOSED_ANGLE 50

#define SERVO_DOOR_OPEN_ANGLE 12
#define SERVO_DOOR_CLOSED_ANGLE 97
#define SERVO_DOOR_LOCKED_ANGLE 125

#define SERVO_DOOR_SETUP_DOWN_ANGLE 37




#include <Servo.h>

Servo trapServo;
Servo doorServo;

void setup() {
  pinMode(SERVO_1_PIN, OUTPUT);
  pinMode(SERVO_2_PIN, OUTPUT);
  pinMode(ENABLE_6V_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, LOW);

  Serial.begin(57600);

  Serial.println("Resetting servos");
  trapServo.attach(SERVO_1_PIN);
  trapServo.write(SERVO_TRAP_CLOSED_ANGLE);
  doorServo.attach(SERVO_2_PIN);
  doorServo.write(SERVO_DOOR_SETUP_DOWN_ANGLE);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(2000); //Give time for servos to move to there start position
  digitalWrite(ENABLE_6V_PIN, LOW);
  Serial.println("Finished resettings servos. They should be in there initial position now.");
  delay(5000);
  //while(true){}
  Serial.println("Finished init.");
}


void moveServo(Servo s, int angle, int mill) {
  s.write(angle);
  digitalWrite(ENABLE_6V_PIN, HIGH);
  delay(mill);
  digitalWrite(ENABLE_6V_PIN, LOW);
}

void loop() {
  /*
  Serial.println("Trigger trap");
  moveServo(trapServo, SERVO_TRAP_OPEN_ANGLE, 1000);
  moveServo(trapServo, SERVO_TRAP_CLOSED_ANGLE, 1000);
  */

  Serial.println("Open door");
  moveServo(doorServo, SERVO_DOOR_OPEN_ANGLE, 2000);
  delay(2000);
  
  Serial.println("Close door");
  moveServo(doorServo, SERVO_DOOR_CLOSED_ANGLE, 2000);
  delay(2000);

  Serial.println("Lock door");
  moveServo(doorServo, SERVO_DOOR_LOCKED_ANGLE, 2000);
  delay(2000);

  delay(5000);
}
