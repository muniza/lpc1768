#ifndef _CONTROLLER_H
#define _CONTROLLER_H

#include "mbed.h"
#include "IVSensors.h"
#include "MotorDriver.h"
#include "Tachometer.h"


class Controller {

public:
    
    Controller(MotorDriver* driver, Tachometer* tachometer, IVSensors* sensors, unsigned int period_us, float step, float outMin, float outMax, float vMax, float iMax, float pMax, float sMax);
    static void changeLimits(float outMin, float outMax, float vMax, float iMax, float pMax, float sMax);
    void period(unsigned int period_us);   
    void enable(char mode);
    void disable();
    void setPoint(float setPoint, char mode);
    void updateInputs();
    void output();

    
private:
    static int    numActive;
    static float  step;
    static float  outMin;
    static float  outMax;
    static float  vMax;
    static float  iMax;
    static float  pMax;
    static float  sMax;

    unsigned int  _period_us;
    float         _period_s;

    float         _setPoint;
    int           _type;

    MotorDriver*  _driver;
    Tachometer*   _tach;
    IVSensors*    _sensors;
    Ticker        _ticker;
    bool          _isEnabled;
};

#endif