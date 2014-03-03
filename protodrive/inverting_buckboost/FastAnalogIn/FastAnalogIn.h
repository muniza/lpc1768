#ifndef FASTANALOGIN_H
#define FASTANALOGIN_H

/*
 * Includes
 */
#include "mbed.h"
#include "pinmap.h"

#ifndef TARGET_LPC1768
    #error "Target not supported"
#endif

/** A class that is similar to AnalogIn, only faster
*
* AnalogIn does a single conversion when you read a value (actually several conversions and it takes the median of that).
* This library has all used ADC channels running automatically in the background, and if you read a value it will immediatly return the last converted value.
*
* You can use several FastAnalogIn objects per ADC pin, and several ADC pins at the same time. Using more ADC pins will decrease the conversion rate.
* If you need to generally convert one pin very fast, but sometimes also need to do AD conversions on another pin, you can disable the ADC channel and still read its value.
* Only now it will block until conversion is complete, so more like the regular AnalogIn library. Of course you can also disable an ADC channel and enable it later.
*
* It does not play nicely with regular AnalogIn objects, so use only this library or only AnalogIn. Also currently only LPC1768 is supported.
*/
class FastAnalogIn {

public:
     /** Create a FastAnalogIn, connected to the specified pin
     *
     * @param pin AnalogIn pin to connect to
     * @param enabled Enable the ADC channel (default = true)
     */
    FastAnalogIn( PinName pin, bool enabled = true );
    
    ~FastAnalogIn( void );
    
    /** Enable the ADC channel
    *
    * @param enabled Bool that is true for enable, false is equivalent to calling disable
    */
    void enable(bool enabled = true);
    
    /** Disable the ADC channel
    *
    * Disabling unused channels speeds up conversion in used channels. 
    * When disabled you can still call read, that will do a single conversion (actually two since the first one always returns 0 for unknown reason).
    * Then the function blocks until the value is read. This is handy when you sometimes needs a single conversion besides the automatic conversion
    */
    void disable( void );
    
    /** Returns the raw value
    *
    * @param return Unsigned integer with converted value
    */
    unsigned short read_u16( void );
    
    /** Returns the scaled value
    *
    * @param return Float with scaled converted value to 0.0-1.0
    */
    float read( void );
    
    /** An operator shorthand for read()
    */
    operator float() {
        return read();
    }

    
private:
    static const PinMap PinMap_ADC[9];
    static int channel_usage[8];
    
    bool running;    
    char ADCnumber;
    uint32_t *datareg;
    
    


};

#endif