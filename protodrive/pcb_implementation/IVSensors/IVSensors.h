#ifndef IVSENSORS_H
#define IVSENSORS_H
#include "mbed.h"
#include "AnalogMux.h"

#define CALIBRATION_SAMPLES 1024

class IVSensors {
public:
    IVSensors(int current, int voltsPlus, int voltsMinus, int numSamples, float voltageMultiplier, int voltageOffset, float currentMultiplier, int currentOffset);
    void calibrate(float voltageMultiplier, int voltageOffset, float currentMultiplier, int currentOffset);
    void calibrateZero();
    float current();
    float voltage();
    float power();
    void update();
private:
    AnalogMux _currentSensor;
    AnalogMux _voltsPlus;
    AnalogMux _voltsMinus;
    int _numSamples;
    int* _currentBuffer;
    int* _voltageBuffer;
    bool _full;
    int _oldest;
    float _voltage;
    bool* isCurrEliminated;
    bool* isVoltEliminated;
    float _current;
    int currentI;
    int voltageI;
    int _currentOffset;
    float _currentMultiplier;
    int _voltageOffset;
    float _voltageMultiplier;
    static void markMinMax(int arr[], bool isElim[], int length);
};

#endif