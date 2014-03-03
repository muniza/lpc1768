#include "FastPWM.h"

FastPWM::FastPWM(PinName pin) : PWMObject(pin){
    //Set clock source
    LPC_SC->PCLKSEL0|=1<<12;
    
    _duty=0;
    _period=0.02;
    if (pin==p26||pin==LED1) {
        PWMUnit=1;
        MR=&LPC_PWM1->MR1;
        }
    else if (pin==p25||pin==LED2){
        PWMUnit=2;
        MR=&LPC_PWM1->MR2;
        }
    else if (pin==p24||pin==LED3){
        PWMUnit=3;
        MR=&LPC_PWM1->MR3;
        }
    else if (pin==p23||pin==LED4){
        PWMUnit=4;
        MR=&LPC_PWM1->MR4;
        }
    else if (pin==p22){
        PWMUnit=5;
        MR=&LPC_PWM1->MR5;
        }
    else if (pin==p21){
        PWMUnit=6;
        MR=&LPC_PWM1->MR6;
        }
    else
        error("No hardware PWM pin\n\r");
    
    period(_period);
}

void FastPWM::period(double seconds) {
    LPC_PWM1->MR0 = (unsigned int) (seconds * (double)F_CLK);
    LPC_PWM1->LER |= 1;
    _period = seconds;
    pulsewidth(_duty*_period);
}

void FastPWM::period_ms(int ms) {
    period((double)ms/1000.0);
}

void FastPWM::period_us(int us) {
    period((double)us/1000000.0);
}

void FastPWM::period_us(double us) {
    period(us/1000000.0);
}

void FastPWM::pulsewidth(double seconds) {
    *MR=(unsigned int) (seconds * (double)F_CLK);
    LPC_PWM1->LER |= 1<<PWMUnit;
    _duty=seconds/_period;
}

void FastPWM::pulsewidth_ms(int ms) {
    pulsewidth((double)ms/1000.0);
}

void FastPWM::pulsewidth_us(int us) {
    pulsewidth((double)us/1000000.0);
}

void FastPWM::pulsewidth_us(double us) {
    pulsewidth(us/1000000.0);
}

void FastPWM::write(double duty) {
        if (duty <= 0.0) _duty = 0.0;
        else if (duty >= 1.0) _duty = 1.0;
        else _duty=duty;
        pulsewidth(_duty*_period);
}

double FastPWM::read( void ) {
    return _duty;
    }
    
FastPWM & FastPWM::operator= (double value) {
    write(value);
    return(*this);
    }
    
FastPWM::operator double() {
    return _duty;
}
    