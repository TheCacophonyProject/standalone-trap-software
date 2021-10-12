#include <Servo.h> 
Servo s;
#define SERVO_PIN 5   //SERVO 1
//#define SERVO_PIN 11  //SERVO 2
#define ENABLE_6V_PIN 4
/*
    
25KG - 180 deg servo

All movements from horizontal, rotate from horizontal (zero)

- move -40deg +5deg ratchet lift (22x)

- hold at +5deg (unloaded)
- release with +35deg

- hold at -40deg

there are no position sensors, the ratchet will just lift to the end of the mechanical device and cannot lift any longer, the release should drop the full length and be able to reset.
*/
// Going to work in uS instead of degrees, sorry.

// Set TEST to true/false and set the angle. Doing this will not go through the sequence and just set the servo to that angle.
#define TEST false
#define TEST_ANGLE 700


// Move from 
#define RATCHET_POS_1 2000
// to  
#define RATCHET_POS_2 1500
#define RATCHET_TIMES 22
// times to lift the ratchet 
// Then turn servo power of and wait
#define WAIT_1_MS 3000 // milliseconds
// Then release by moving to 
#define RATCHET_POS_3 1000
// then wait
#define WAIT_2_MS 2000 // milliseconds
// then hold at RATCHET_POS_1 waiting for sequence to restart


void setup() {
  // put your setup code here, to run once:
  pinMode(ENABLE_6V_PIN, OUTPUT);
  digitalWrite(ENABLE_6V_PIN, LOW);
  s.attach(SERVO_PIN, 500, 2500);
  Serial.begin(57600);
  Serial.println("Finished init");
  while (TEST) {
    digitalWrite(ENABLE_6V_PIN, HIGH);
    s.writeMicroseconds(RATCHET_POS_1);
    delay(1000);
  }
}

void loop() {
  Serial.println("Starting sequence");
  digitalWrite(ENABLE_6V_PIN, HIGH);
  for (int i = 1; i <= RATCHET_TIMES; i++) {
    Serial.print("ratchet count: ");
    Serial.println(i);
    s.writeMicroseconds(RATCHET_POS_1);
    delay(500);
    s.writeMicroseconds(RATCHET_POS_2);
    delay(500);
  }
  Serial.println("Waiting to trigger");
  digitalWrite(ENABLE_6V_PIN, LOW);
  delay(WAIT_1_MS);
  Serial.println("Triggering");
  digitalWrite(ENABLE_6V_PIN, HIGH);
  s.writeMicroseconds(RATCHET_POS_3);
  delay(WAIT_2_MS);
  s.writeMicroseconds(RATCHET_POS_1);
  delay(1000);
  digitalWrite(ENABLE_6V_PIN, LOW);
  
  
  // put your main code here, to run repeatedly:
  Serial.println("Finished loop. Waiting a bit");
  delay(5000);
  Serial.println("");
}
