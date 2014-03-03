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
 
// --------------------- Median Filtered Linear Temperature Sensor Reader ----------------------------
 
#include "LinearAnalogSensor.h"
#include "mbed.h"

// Constructor using two calibration points to define linear multiplier and offset
LinearAnalogSensor::LinearAnalogSensor(PinName pin, int numSamples, float temp1, unsigned short read1, float temp2, unsigned short read2):_pin(pin) {
    _reading = 0.0;                                     // Zero the temperature
    _multiplier = (temp2 - temp1) / (read2 - read1); // Calculate multiplier as slope
    _offset = temp1 - (_multiplier * read1);         // Calculate offset
    _numSamples = numSamples;
    _arr = new unsigned short[numSamples];
}
// Constructor using user defined multiplier and offset
LinearAnalogSensor::LinearAnalogSensor(PinName pin, int numSamples, float multiplier, float offset):_pin(pin) {
    _reading = 0.0;                                     // Zero the temperature
    _multiplier = multiplier;                        // Set multiplier
    _offset = offset;                                // Set offset
    _numSamples = numSamples;
    _arr = new unsigned short[numSamples];
}

// Populates an array with temperature readings, sorts, and returns median
float LinearAnalogSensor::read() {
    memset(_arr, 0, _numSamples * sizeof(unsigned short));
    _arr[0] = _pin.read_u16();
    for (int i = 1; i < _numSamples; i++) { 
        _arr[i] = _pin.read_u16();
        unsigned short tmp = _arr[i];
        int j;            
        for (j = i - 1; j >= 0; j--) {
            if (tmp>=_arr[j]) break;
            _arr[j + 1] = _arr[j];
        }
        _arr[j + 1] = tmp;
    }
    if (_numSamples % 2 == 0) {
        _reading = (((_arr[_numSamples >> 1] + _arr[(_numSamples >> 1) - 1]) / (2.0)) * _multiplier) + _offset;
    }
    if (_numSamples % 2 == 1)  {
        _reading = (_arr[_numSamples >> 1] * _multiplier) + _offset;
    }
    return _reading;
}
// Returns last calculated temperature value
float LinearAnalogSensor::getLast() {
    return _reading;
}