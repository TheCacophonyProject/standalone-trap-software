#ifndef linearActuator_h
#define linearActuator_h

class LinearActuator {
  public:
    LinearActuator(int pwmPin, int sensePin, int forwardPin, int backPin);
    void setup();
    void init();
    void forward();
    void back();
    void rampPWMOn();
    void rampPWMOff();
  private:
    void waitForNoCurrent();
    int _pwmPin;
    int _sensePin;
    int _forwardPin;
    int _backPin;
};

#endif
