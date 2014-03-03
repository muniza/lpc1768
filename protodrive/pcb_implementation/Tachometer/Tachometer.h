#ifndef TACH_DEFH
#define TACH_DEFH
#include "mbed.h"

/*
 * This library uses the mbed library InterruptIn and Timer interfaces to read a tachometer pulse
 * train.  The signal may define the rotation of a wheel, motor, etc.  The amount of pulses per
 * revolution (ppr), the amount of pulses per conversion (ppc), the pin, and a timeout period length
 * are given to the constructor.  The library is more prone to reporting inconsistent readings
 * given a lower ppc due to inconsistencies in the pulses of the tachometer signal.
 */
 
 #define RISING 0
 #define FALLING 1
 
 class Tachometer {
 
 public:
    Tachometer(PinName pin, int ppr, int ppc, float timeOut, int polarity);
    float rpm();                    // Returns rotations per minute
    unsigned int rotation_us();     // Returns microseconds per rotation
    void enable();                  // Start interrupt driven background counter
    void disable();                 // Stop interrupt driven background counter
    
    #ifdef MBED_OPERATORS

    operator float() {
        return rpm();
    }
    
    #endif
    
    
 private:
    void timeOutFunc();             // Executed on timeout (no pulses for a while)
    void irq();                     // Executed on interrupt
    InterruptIn _pin;               // Pin to which tachometer signal is connected
    Timer _timer;                   // Timer object which tracks signal duration and pulses
    Timeout _timeOut;               // Timeout to stop waiting on signal and report 0 speed
    unsigned int _timeOut_us;       // Timeout interval in microseconds
    int _ppr;                       // Pulses per revolution
    int _ppc;                       // Pulses per conversion
    unsigned int _lastKnown_us;     // Last computed duration of one rotation
    int _numSamples;                // Number of edges captured since last conversion
    bool _isRising;                 // Flag to mark whether to count rising or falling edges
 };
 
#endif