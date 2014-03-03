#include "mbed.h"
#include "AnalogMux.h"
#include "IVSensors.h"

IVSensors::IVSensors(int current, int voltsPlus, int voltsMinus, int numSamples, float voltageMultiplier, int voltageOffset, float currentMultiplier, int currentOffset): _currentSensor(current), _voltsPlus(voltsPlus), _voltsMinus(voltsMinus) {

    // Set parameters
    _numSamples = numSamples;
    _voltage = _current = 0;
    _voltageOffset = voltageOffset;
    _currentOffset = currentOffset;
    _voltageMultiplier = voltageMultiplier;
    _currentMultiplier = currentMultiplier;
    
    // Allocate the working arrays
    _currentBuffer = new int[numSamples];
    memset(_currentBuffer, 0, sizeof(int) * numSamples);    
    _voltageBuffer = new int[numSamples];
    memset(_voltageBuffer, 0, sizeof(int) * numSamples);
    _oldest = 0;
    _full = false;
    
    isCurrEliminated = new bool[_numSamples];
    isVoltEliminated = new bool[_numSamples];
    
    memset(isCurrEliminated, 0, sizeof(bool) * _numSamples);
    memset(isVoltEliminated, 0, sizeof(bool) * _numSamples);

}

void IVSensors::calibrate(float voltageMultiplier, int voltageOffset, float currentMultiplier, int currentOffset) {
    
    // Set parameters
    _voltageMultiplier = voltageMultiplier;
    _voltageOffset = voltageOffset;
    _currentMultiplier = currentMultiplier;
    _currentOffset = currentOffset;
    
}

void IVSensors::calibrateZero() {
    long current = 0;
    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        current += _currentSensor.read_u16();
    }
    current /= CALIBRATION_SAMPLES;

    // Set new offset (b in y=m(x+b))
    _currentOffset = -1 * current;
}

// Apply linear transformation to get final float and store
float IVSensors::current() {
    _current = (currentI + _currentOffset) * _currentMultiplier;
    //printf("currentI: %d co: %d cmult: %f result %f\r\n", currentI, _currentOffset, _currentMultiplier, _current);

    return _current;
}

// Apply linear transformation to get final float and store
float IVSensors::voltage() {
    _voltage = (voltageI + _voltageOffset) * _voltageMultiplier;
    //printf("voltageI: %d vo: %d vmult: %f result %f\r\n", voltageI, _voltageOffset, _voltageMultiplier, _voltage);
    return _voltage;
}

float IVSensors::power() {
    return (_current * _voltage);
}

void IVSensors::markMinMax(int arr[], bool isElim[], int length) {
    
    // Find and mark min
    int min = 65535;
    int index = 0;
    for (int i = 0; i < length; i++) {
        if (isElim[i]) continue;
        if (arr[i] <= min) {
            min = arr[i];
            index = i;
        }
    }
    isElim[index] = true;
    
    // Find and mark max
    int max = -65535;
    index = 0;
    for (int i = 0; i < length; i++) {
        if (isElim[i]) continue;
        if (arr[i] >= max) {
            max = arr[i];
            index = i;
        }
    }
    isElim[index] = true;
}

void IVSensors::update() {

    // First time, when buffers are empty - fill up the buffers with newly read values
    if (!_full) {
        for (int i = 0; i < _numSamples; i++) {
            _currentBuffer[i] = _currentSensor.read_u16();
            _voltageBuffer[i] = _voltsPlus.read_u16() - _voltsMinus.read_u16();
        }
        _full = true;
        
    // All other times - replace the oldest value with a newly read value
    } else {
        _currentBuffer[_oldest] = _currentSensor.read_u16();
        _voltageBuffer[_oldest] = _voltsPlus.read_u16() - _voltsMinus.read_u16();
        _oldest++;
        if (_oldest >= _numSamples) _oldest = 0;
    }
    
/*
    for (int i = 0; i < _numSamples; i++) {
        isCurrEliminated[i] = false;
        isVoltEliminated[i] = false;
    }
  */  
    long sumV = 0;
    long sumI = 0;
    for (int i = 0; i < _numSamples; i++) {
        sumV += _voltageBuffer[i];
        sumI += _currentBuffer[i];
    }
    currentI = (long)(sumI / (long)(_numSamples));
    voltageI = (long)(sumV / (long)(_numSamples));
    
    /*
    // Compute median by eliminating min and max multiple times
    int times = 0;
    if (_numSamples % 2) times = _numSamples >> 1;
    else times = (_numSamples >> 1) - 1;
    

    // Eliminate min and max for numSamples / 2 times for current buffer
    for (int i = 0; i < times; i++) {
        IVSensors::markMinMax(_currentBuffer, isCurrEliminated, _numSamples);
    }
    
    // Eliminate min and max for numSamples / 2 times for voltage buffer
    for (int i = 0; i < times; i++) {
        IVSensors::markMinMax(_voltageBuffer, isVoltEliminated, _numSamples);
    }
    
    // Find the un-eliminated values, must be the median for an odd numSamples
    int i;
    
    if (_numSamples % 2) {
        for (i = 0; i < _numSamples; i++) {
            if (!isCurrEliminated[i]) break;
        }
        currentI = _currentBuffer[i];
    
        for (i = 0; i < _numSamples; i++) {
            if (!isVoltEliminated[i]) break;
        }
        voltageI = _voltageBuffer[i];
        
    // Find the two uneliminated values and take the average
    } else {
        for (i = 0; i < _numSamples; i++) {
            if (!isCurrEliminated[i]) break;
        }
        int s1 = _currentBuffer[i];
        for (; i < _numSamples; i++) {
            if (!isCurrEliminated[i]) break;
        }
        int s2 = _currentBuffer[i];
        currentI = (int)((unsigned int)(s1 + s2) >> 1);
        
        for (i = 0; i < _numSamples; i++) {
            if (!isVoltEliminated[i]) break;
        }
        s1 = _voltageBuffer[i];
        for (; i < _numSamples; i++) {
            if (!isVoltEliminated[i]) break;
        }
        s2 = _voltageBuffer[i];
        voltageI = (int)((unsigned int)(s1 + s2) >> 1);
    }

    memset(isCurrEliminated, 0, sizeof(bool) * _numSamples);
    memset(isVoltEliminated, 0, sizeof(bool) * _numSamples);*/
}