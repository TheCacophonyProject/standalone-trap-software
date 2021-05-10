#include "linearActuator.h"
#include "util.h"
#include "Config.h"

LinearActuator::LinearActuator(int pwmPin, int sensePin, int forwardPin, int backPin) {
  _pwmPin = pwmPin;
  _sensePin = sensePin;
  _forwardPin = forwardPin;
  _backPin = backPin;
}

void LinearActuator::setup() {
  pinMode(_sensePin, INPUT);
  pinMode(_forwardPin, OUTPUT);
  pinMode(_backPin, OUTPUT);
  pinMode(_pwmPin, OUTPUT);
  digitalWrite(_forwardPin, LOW);
  digitalWrite(_backPin, LOW);
  digitalWrite(_pwmPin, LOW);
}

void LinearActuator::init() {
  Serial.print("Running LinearActuator init...  ");
  Serial.println("Done.");
}

void LinearActuator::reset() {
  Serial.println("Resetting trap");
  digitalWrite(_forwardPin, LOW);
  digitalWrite(_backPin, LOW);
  delay(100);

  Serial.println("Moving Linear actuator back");
  digitalWrite(_pwmPin, LOW);
  digitalWrite(_backPin, HIGH);
  delay(100);
  rampPWMOn();
  waitForNoCurrent();
  rampPWMOff();

  Serial.println("Moving Linear actuator forward");
  digitalWrite(_backPin, LOW);
  digitalWrite(_forwardPin, HIGH);
  delay(100);
  rampPWMOn();
  waitForNoCurrent();
  rampPWMOff();

  Serial.println("Moving Linear actuator back");
  digitalWrite(_backPin, HIGH);
  digitalWrite(_forwardPin, LOW);
  delay(100);
  rampPWMOn();
  waitForNoCurrent();
  rampPWMOff();

  Serial.println("Finished resetting trap");
}

void LinearActuator::waitForNoCurrent() {
  int count = 0;
  while (count < 10) {
    if (analogRead(_sensePin) == 0) {
      count++;
    } else {
      count = 0;
    }
  }
}

void LinearActuator::rampPWMOn() {
  int i = 0;
  while (i < 255) {
    analogWrite(_pwmPin, i);
    delay(2);
    i++;
  }
  digitalWrite(_pwmPin, HIGH);
}

void LinearActuator::rampPWMOff() {
  int i = 255;
  while (i > 0) {
    analogWrite(_pwmPin, i);
    delay(2);
    i--;
  }
  digitalWrite(_pwmPin, LOW);
}
