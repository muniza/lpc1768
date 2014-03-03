#include "mbed.h"
#include "Tachometer.h"
 
#define US_PER_SECOND 1000000

DigitalOut led3(LED3);
 
Tachometer::Tachometer(PinName pin, int ppr, int ppc, float timeOut, int polarity):_pin(pin) {
    _ppr = ppr;
    _ppc = ppc;
    _timeOut_us = timeOut * US_PER_SECOND;
    _lastKnown_us = 0;
    if (polarity == RISING) _isRising = true;
    else if (polarity == FALLING) _isRising = false;
    else return;
    
    NVIC_SetPriority(EINT3_IRQn, 1);
    NVIC_SetPriority(TIMER3_IRQn, 2);
}

void Tachometer::timeOutFunc() {
    _lastKnown_us = 0;
}

void Tachometer::irq() {
    _timeOut.detach();
    if (_numSamples == -1) {
        _timer.reset();
        _numSamples = 0;
    }
    _numSamples++;
    if (_numSamples == _ppc) {
        _lastKnown_us = (unsigned int)(_timer.read_us());
        _timer.reset();
        _numSamples = 0;
    }
    _timeOut.attach_us(this, &Tachometer::timeOutFunc, _timeOut_us);
}

float Tachometer::rpm() {
    unsigned int temp = rotation_us();
    if (temp == 0.0) return 0.0;
    return ((US_PER_SECOND * 60.0) / temp);
}

unsigned int Tachometer::rotation_us() {
    return _lastKnown_us * (_ppr / _ppc);
}

void Tachometer::enable() {
    if (_isRising) _pin.rise(this, &Tachometer::irq);
    else _pin.fall(this, &Tachometer::irq);
    _timer.start();
}

void Tachometer::disable() {
    if (_isRising) _pin.rise(NULL);
    else _pin.fall(NULL);
    _timer.stop();
}