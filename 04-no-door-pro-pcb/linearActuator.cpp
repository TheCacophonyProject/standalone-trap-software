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
  digitalWrite(_forwardPin, HIGH);
  digitalWrite(_backPin, HIGH);
  digitalWrite(_pwmPin, LOW);
  TCCR1B = TCCR1B & B11111000 | B00000001;    // set timer 1 divisor to     1 for PWM frequency of 31372.55 Hz for D9 and D10
}

void LinearActuator::init() {
  Serial.print("Running LinearActuator init...  ");
  Serial.println("Done.");
}

void LinearActuator::back() {
  Serial.println("Moving Linear actuator back");
  digitalWrite(_forwardPin, HIGH);
  digitalWrite(_backPin, HIGH);
  delay(100);
  digitalWrite(_pwmPin, LOW);
  digitalWrite(_backPin, LOW);
  delay(100);
  rampPWMOn();
  waitForNoCurrent();
  rampPWMOff();
  digitalWrite(_forwardPin, HIGH);
  digitalWrite(_backPin, HIGH);
}


void LinearActuator::forward() {
  Serial.println("Moving Linear actuator forward");
  digitalWrite(_forwardPin, HIGH);
  digitalWrite(_backPin, HIGH);
  delay(100);
  digitalWrite(_pwmPin, LOW);
  digitalWrite(_forwardPin, LOW);
  delay(100);
  rampPWMOn();
  waitForNoCurrent();
  rampPWMOff();
  digitalWrite(_forwardPin, HIGH);
  digitalWrite(_backPin, HIGH);
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
