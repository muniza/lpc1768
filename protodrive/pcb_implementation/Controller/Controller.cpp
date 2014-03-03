#include "mbed.h"
#include "IVSensors.h"
#include "MotorDriver.h"
#include "Tachometer.h"
#include "Controller.h"

float Controller::outMin  = 0.0;
float Controller::outMax  = 0.0;
float Controller::vMax    = 0.0;
float Controller::iMax    = 0.0;
float Controller::pMax    = 0.0;
float Controller::sMax    = 0.0;
float Controller::step    = 0.0;
int Controller::numActive = 0;
const float Krpm = 1.0;

DigitalOut led4(LED4);
DigitalOut led2(LED2);

Controller::Controller(MotorDriver* driver, Tachometer* tachometer, IVSensors* sensors, unsigned int period_us, float step, float outMin, float outMax, float vMax, float iMax, float pMax, float sMax) {
    _driver = driver;
    _tach = tachometer;
    _sensors = sensors;
    _isEnabled = false;
    _period_us = period_us;
    _period_s = (float)(period_us / 1000000.0);
        
    changeLimits(outMin, outMax, vMax, iMax, pMax, sMax);
    Controller::step = step;

    _setPoint = 0;
    _type = 0;
}

void Controller::changeLimits(float outMin, float outMax, float vMax, float iMax, float pMax, float sMax) {
    Controller::outMin = outMin;
    Controller::outMax = outMax;
    Controller::vMax = vMax;
    Controller::iMax = iMax;
    Controller::pMax = pMax;
    Controller::sMax = sMax;
}    
void Controller::period(unsigned int period_us) {
    _period_us = period_us;
    _period_s = period_us / 1000000.0;
    if (_isEnabled) {
        _ticker.detach();
        _ticker.attach_us(this, &Controller::output, _period_us);
    }
}
void Controller::enable(char mode) {
    _driver->enable();
    _tach->enable();
    numActive++;
    if (mode == 'a') {
        _ticker.attach_us(this, &Controller::output, _period_us);
        _isEnabled = true;
    }
}
void Controller::disable() {
    numActive--;
    _ticker.detach();
    _isEnabled = false;
    _driver->disable();
    
    if (numActive <= 0) _tach->disable();
    _setPoint = 0;
    _type = 0;
}
void Controller::setPoint(float setPoint, char mode) {
    if (mode == 'v') {
        _type = 0;
        if (setPoint > vMax) setPoint = vMax;
        if (setPoint < 0) setPoint = 0;
    } if (mode == 'i') {
        _type = 1;
        if (setPoint > iMax) setPoint = iMax;
        if (setPoint < -1 * iMax) setPoint = -1 * iMax;
    } if (mode == 'p') {
        _type = 2;
        if (setPoint > pMax) setPoint = pMax;
        if (setPoint < -1 * pMax) setPoint = -1 * pMax;
    } if (mode == 's') {
        _type = 3;
        if (setPoint > sMax) setPoint = sMax;
        if (setPoint < 0) setPoint = 0;
    }
    _setPoint = setPoint;
}

void Controller::output() {
    float pv = 0.0;
    
    float current = _sensors->current();
    float voltage = _sensors->voltage();
    float power = _sensors->power();

    float resistance = (voltage / current);
    if (resistance > 15 || resistance < 7) resistance = 8.0;
    float rpm = voltage - (current * resistance) * Krpm;
    /*
    if (voltage >= vMax || fabs(current) >= iMax || fabs(power) >= pMax || rpm >= sMax) {
        _driver->setDuty(_driver->readDutyF() - step);
        led4=!led4;
        printf("Over param\r\n");
        return;
    }*/
    if (_type == 0) {
        pv = voltage;
    } if (_type == 1) {
        pv = current;
    } if (_type == 2) {
        pv = power;
    } if (_type == 3) {
        pv = rpm;
    }
    
    float currentDuty = _driver->readDutyF();
    
    if (pv < _setPoint) {
        _driver->setDuty(currentDuty + step);
    }
    if (pv > _setPoint) {
        _driver->setDuty(currentDuty - step);
    }
    led2=!led2;
    /*
    float error = _setPoint - pv;
    float derivative = ((error - _previousError) / _period_s);
    if (currentDuty < (outMax - (0.05 * (outMax - outMin))) && currentDuty > (outMin + (0.05 * (outMax - outMin)))) _accumulatedError += (error * _period_s);
    
    float output = bias[_type] + (error * Kp[_type]) + (_accumulatedError * Ki[_type]) + (derivative * Kd[_type]);
    
    _driver->setDuty(_driver->readDutyF() + output); 
    _previousError = error;
  */
  
    printf("\r\nPV: %f Out: %f Setpoint: %f Type: %d\r\n", pv, _driver->readDutyF(), _setPoint, _type);
}

void Controller::updateInputs() {
    _sensors->update();
}