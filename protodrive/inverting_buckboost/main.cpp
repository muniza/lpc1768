#include "mbed.h"
#include "FastAnalogIn.h"
#include "FastPWM.h"
#include "LinearAnalogSensor.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);
//AnalogIn sense(p20);

DigitalOut extra1(p19);
DigitalOut extra2(p18);
DigitalOut extra3(p17);
DigitalOut extra5(p15);
FastPWM sw(p21);
FastPWM chargePump(p22);
Serial pc(USBTX, USBRX);
Ticker printer;
LinearAnalogSensor sense(p20, 33, 0.00006828308, -2.527973+0.048);
LinearAnalogSensor volts(p16, 33, ((1.0/65536.0) * 3.3 * 6.453110971), 0);

const float step = 0.003;
const double currStep = 0.01;
const long frequency = 20000;
float setPoint = 0;
bool printing = false;
long times = 0;
float avgCurr = 0;
float avgVolts = 0;

void print() {
    printing = true;
}

float getCurrent() {
    float curr = sense;
    return curr;
}

void setDuty(char dir) {
    if (dir == '+') sw = sw - step;
    else sw = sw + step;
}

void update() {
    float current = getCurrent();
    float voltage = volts;
    float power = current * volts;
    avgCurr = avgCurr + current;
    avgVolts = avgVolts + voltage;
    if (power < setPoint) setDuty('+');
    else setDuty('-');
    times++;
}

int main() {
    printer.attach(&print, 0.5);
    pc.baud(115200);
    chargePump.period(1.0/frequency);
    chargePump = 0.5;
    sw.period(1.0/frequency);
    sw = 1.0;
    
    wait(1);
    
    while (1) {
        if (printing) {
            double curr = avgCurr / times;
            avgCurr = 0;
            double volts = avgVolts / times;
            avgVolts = 0;
            double watts = curr * volts;
            double dty = 1.0 - sw;
            pc.printf("Dty: %lf Voltage: %lfV Current: %lfA  Power: %lfW Setpoint: %lfW Times run: %ld\r\n", dty, volts, curr, watts, setPoint, times);
            times =  0;
            printing = false;
        }
        if (pc.readable()) {
            char c = pc.getc();
            if (c == '+') setPoint = setPoint + currStep;
            if (c == '-') setPoint = setPoint - currStep;
            if (setPoint < 0) setPoint = 0;
        }        
        update();
    }
}
