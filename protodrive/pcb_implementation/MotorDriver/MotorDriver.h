#ifndef MO_DRVR_H
#define MO_DRVR_H

/* Configure and use a complimentary, Half-H Bridge PWM Driver pair with variable deadtime insertion 
 * to eliminate power supply shoot through. The library uses the LPC17xx PWM1 and Timer2 interfaces to generate a max of 6
 * signals (3 pairs). You should not create or use any standard mbed-library PWM objects when using this
 * library because timings will be affected.
 */
 
#include "mbed.h"

class MotorDriver {

public:
    MotorDriver(PinName pin, uint32_t frequency, uint32_t deadTicks);
    void setFreq(uint32_t frequency);
    uint32_t getFreq();
    void setDuty(int duty);
    void setDuty(float duty);
    float readDutyF();
    int readDuty();
    void setDeadTicks(uint32_t deadTicks);
    uint32_t getDeadTicks();
    uint32_t getMaxDuty();
    void disable();
    void enable();
    
    static uint32_t periodTicks;
    static uint32_t Timer2MR[3];
    static bool isEnabled[3];
    
    #ifdef MBED_OPERATORS

    operator int() {
        return readDuty();
    }
    
    MotorDriver& operator=(int value) {
        setDuty(value);
        return *this;
    }
    
    MotorDriver& operator=(MotorDriver& other) {
        setDuty(other.readDuty());
        return *this;
    }
    #endif

private:
    int _pinPair;
    int _dutyTicks;
    uint32_t _minDutyTicks;
    uint32_t _maxDutyTicks;
    uint32_t _deadTicks;
    
};

#endif