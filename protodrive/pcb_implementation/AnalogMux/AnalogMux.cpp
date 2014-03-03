#include "mbed.h"
#include "AnalogMux.h"

static const PinName arr[]  = {p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30};
static const char portArr[] = { 0 , 0 , 0 , 0 , 0  , 0  , 0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   2,   2,   2,   2,   2,   2,   0,   0,   0,   0};
static const char pin[]     = { 9 , 8 , 7 , 6 , 0  , 1  ,18,  17,  15,  16,  23,  24,  25,  26,  30,  31,   5,   4,   3,   2,   1,   0,  11,  10,   5,   4};

AnalogIn* AnalogMux::analog = 0;
PortOut* AnalogMux::port = 0;
uint32_t AnalogMux::disableWrite = 0;
uint32_t AnalogMux::selection[16] = {0};

// Helper function to convert PinName pin to an int which is used for port and pin lookup
static int pinName2int(PinName pin) {
    int tmp;
    for (tmp = 0; tmp < 26; tmp++) {
        if (pin == arr[tmp]) break;
    }
    if (tmp >= 26) return -1;
    return tmp;
}

void AnalogMux::MuxSetup(PinName input, PinName EN, PinName S0, PinName S1, PinName S2, PinName S3) {

    analog = new AnalogIn(input);
    int ENz = pinName2int(EN);
    int S0z = pinName2int(S0);
    int S1z = pinName2int(S1);
    int S2z = pinName2int(S2);
    int S3z = pinName2int(S3);

    int outPort = portArr[ENz];
    ENz = pin[ENz];
    S0z = pin[S0z];
    S1z = pin[S1z];
    S2z = pin[S2z];
    S3z = pin[S3z];

    uint32_t bitmask = (1 << ENz) | (1 << S0z) | (1 << S1z) | (1 << S2z) | (1 << S3z);
    
    if (outPort == 0) port = new PortOut(Port0, bitmask);
    else if (outPort == 1) port = new PortOut(Port1, bitmask);
    else if (outPort == 2) port = new PortOut(Port2, bitmask);

    for (int i = 0; i < 16; i++) {
        selection[i] = ((i & 1) << S0z) | (((i & 2) >> 1) << S1z) | (((i & 4) >> 2) << S2z) | (((i & 8) >> 3) << S3z);
    }
    disableWrite = 1 << ENz;
}

float AnalogMux::MuxRead(int channel) {
    (*port).write(selection[channel]);
    return ((*analog).read());
}
unsigned short AnalogMux::MuxRead_u16(int channel) {
    (*port).write(selection[channel]);
    return ((*analog).read_u16());
}
void AnalogMux::disable() {
    (*port).write(disableWrite);
}
AnalogMux::AnalogMux(int channel) {
    _channel = channel;
}
float AnalogMux::read() {
    return MuxRead(_channel);
}
unsigned short AnalogMux::read_u16() {
    return MuxRead_u16(_channel);
}