#ifndef ANALOG_MUX_H
#define ANALOG_MUX_H
#include "mbed.h"

/* This library allows an external multiplexer to be connected to the mbed using 4 digital signal switch lines
 * and a single common analog channel.  The user can specify which pins to use, given that they are all part of
 * the same port so they may to written and read in a single operation.  Library specifically made for the 74HC4067 IC.
 * First, the MuxSetup() must be run, then to read channels, new channel-specific AnalogInMux objects can be
 * created and used just like regular AnalogIn, or you can specify a channel to the static MuxRead() function.
 */

class AnalogMux {

public:
    static void MuxSetup(PinName input, PinName EN, PinName S0, PinName S1, PinName S2, PinName S3);
    static float MuxRead(int channel);
    static unsigned short MuxRead_u16(int channel);
    static void disable();
    
    AnalogMux(int channel);
    float read();
    unsigned short read_u16();

    #ifdef MBED_OPERATORS

    operator float() {
        return read();
    }

    #endif

private:
    static AnalogIn* analog;
    static PortOut* port;
    static uint32_t selection[16];
    static uint32_t disableWrite;
    int _channel;
};


#endif