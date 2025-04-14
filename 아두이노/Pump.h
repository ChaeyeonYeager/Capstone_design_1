#ifndef PUMP_H
#define PUMP_H

class PumpController {
  private:
    bool pumpState;  // true: 열림(HIGH), false: 닫힘(LOW)

  public:
    void begin();
    void update();
    bool isPumpOn();
};

#endif
