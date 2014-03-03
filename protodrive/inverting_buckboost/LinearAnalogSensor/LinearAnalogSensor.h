/* Copyright (c) <2012> <P. Patel>, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
 // --------------------- Median Filtered Linear Temperature Sensor Reader ------------------------
 
#ifndef LINEAR_ANALOG
#define LINEAR_ANALOG

#include "mbed.h"
/** This library is designed to work with devices like the LM335 temperature sensor.  There are only
 * two requirements for compatibility of a device with this library: 1) It must be a sensor device
 * that has an analog voltage output. 2) The physical quantity measured must vary linearly with 
 * the analog voltage.  The LM335 which creates an analog voltage proportional to temperature
 * can work very well with this library using the nominal multiplier of 0.0050354 and offset of 
 * -273.15 or by providing two calibration points to the constuctor to generate a linear response.  It
 * samples the sensor 9 times and uses the median to minimize the effect of poor ADC readings and sudden
 * changes. 
 * 
 * Example:
 * @code
 * LinearAnalogSensor mysensor(p20, 0.0050354, -273.15);        // Setup a LM335 temp sensor on pin 20
 * DigitalOut myled(LED1);
 * 
 * int main() {
 *     while(1) {
 *         // Light LED if filtered temperature is greater than 45 degrees Celsius
 *         if (mysensor > 45) myled = 1;
 *         else myled = 0;
 *     }
 * }
 * @endcode
 */
class LinearAnalogSensor {
public:
    /** Create a calibrated temperature sensor object connected to an Analog input pin
     * @param pin Analog input pin to which an LM335 or comparable linear temp sensor is connected
     * @param numSamples Number of samples over which to apply a median filter
     * @param temp1 Temperature (float in degrees C or F) of first calibration point
     * @param read1 Mbed ADC reading (unsigned short) of first calibration point
     * @param temp2 Temperature (float in degrees C or F) of second calibration point
     * @param read2 Mbed ADC reading (unsigned short) of second calibration point
     */
    LinearAnalogSensor(PinName pin, int numSamples, float temp1, unsigned short read1, float temp2, unsigned short read2);
    /** Create a calibrated temperature sensor object connected to an Analog input pin
     * @param pin Analog input pin to which an LM335 or comparable linear temp sensor is connected
     * @param numSamples Number of samples over which to apply a median filter
     * @param multiplier Conversion multiplier to go from ADC reading as unsigned short to temperature (change in degrees / change in unsigned short)
     * @param offset Conversion offset (positive or negative)
     */
    LinearAnalogSensor(PinName pin, int numSamples, float multiplier,  float offset);
    /** Returns a new median-filtered, converted reading from the sensor
      * @returns New temperature (float) in degrees C or F as designated by the calibration points
      */
    float read();
    /** Return the last calculated and stored temperature
      * @returns Last stored temperature (float) in degrees C or F as designated by the calibration points
      */
    float getLast();
    #ifdef MBED_OPERATORS
    /** An operator shorthand for readTemp() to calculate filtered temperature
     */
    operator float() {
        return read();
    }
    #endif
private:
    AnalogIn _pin;
    unsigned short * _arr;
    float _reading;
    float _multiplier;
    int _numSamples;
    float _offset;
};
#endif