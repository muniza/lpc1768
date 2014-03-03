#include "mbed.h"
#include "MotorDriver.h"

#define PCLK_HZ 96000000.0
#define INTERRUPT_DELAY 60


uint32_t MotorDriver::periodTicks = 0;                 // Period shared by PWM and Timer
uint32_t MotorDriver::Timer2MR[3] = {0};               // Most recent values to load into Timer2 match registers
bool MotorDriver::isEnabled[3] = {false};              // Flag to track active MotorDrivers

// Interrupt triggered at the start of every period to reset output and
// reload match registers at the same time PWM updates on its MR0 match
extern "C" void TIMER2_IRQHandler() {

    LPC_TIM2->IR = 0x3f;                                    // Clear any pending timer interrupts  
    
    // Do only if a MotorDriver is still active
    if (MotorDriver::isEnabled[0] || MotorDriver::isEnabled[1] || MotorDriver::isEnabled[2]) {
        LPC_TIM2->MR0 = (MotorDriver::periodTicks) - 1 ;           // Update freq
        LPC_TIM2->EMR =  (1 << 7) | (1 << 9) | (1 << 11);   // Set outputs on match (HIGH), clear them now (LOW)
    
        // Update the match registers with the last stored information
        LPC_TIM2->MR1 = MotorDriver::Timer2MR[0];
        LPC_TIM2->MR2 = MotorDriver::Timer2MR[1];
        LPC_TIM2->MR3 = MotorDriver::Timer2MR[2];    
    } else LPC_TIM2->EMR = 0;    // Disable output
}

// Constructor to initialize a MotorDriver object given pinPair(1, 2, or 3), frequency, and deadTicks (10ns per tick)
MotorDriver::MotorDriver(PinName pin, uint32_t frequency, uint32_t deadTicks) {
    if (pin == p25) _pinPair = 1;
    else if (pin == p23) _pinPair = 2;
    else if (pin == p21) _pinPair = 3;
    else return;
    
    LPC_SC->PCONP |= (1 << 6) | (1 << 22);  // Power on PWM1 and Timer2 Module
    LPC_SC->PCLKSEL0 |= 1 << 12;            // Set PWM peripheral clock scalar to 1  (96Mhz)
    LPC_SC->PCLKSEL1 |= 1 << 12;            // Set Timer2 peripheral clock scalar to 1  (96Mhz)
    LPC_PWM1->IR = 0x3bf;                   // Clear any pending pwm interrupts
    LPC_TIM2->IR = 0xf;                     // Clear any pending timer interrupts
    LPC_PWM1->PR = 0;                       // PWM1 prescalar to 1 (direct from PClK 96Mhz)
    LPC_TIM2->PR = 0;                       // Timer2 prescalar to 1 (direct from PClK 96Mhz)
   
    setFreq(frequency);                     // Set waveform output frequency
    setDeadTicks(deadTicks);                // Set the deadtime between edge transitions

    LPC_PWM1->MCR |= (1 << 1);              // Reset PWM counter on MR0
    LPC_TIM2->MCR |= (1 << 1);              // Reset Timer2 on MR0
    
    setDuty(0);                             // Initialize the match registers for 0 duty output
    
    NVIC_EnableIRQ(TIMER2_IRQn);              // This will enable all interrupte pertaining to Timer2
    NVIC_SetPriority(TIMER2_IRQn, 0);         // Set to highest priority

    LPC_TIM2->MR0 = periodTicks - 1;
    LPC_TIM2->MCR |= 1;                       // Activate timer MR0 interrupt
    LPC_TIM2->TCR = 1;                        // Start timer2 counter
    LPC_PWM1->TCR = (1 << 3) | 1;             // Activate latch enabled reload (LER) pwm mode amd start counter
}

// Change the frequency of all PWM1 and Timer2 outputs, relies on interrupt to change Timer2
// settings at the same time as PWM1 settings are latched in on the next MR0 match event
void MotorDriver::setFreq(uint32_t frequency) {

    periodTicks = PCLK_HZ / frequency;                // Store to globally accessable variable
    _maxDutyTicks = periodTicks;                      // Set the maximum number of duty ticks for Timer2 MR1
    _minDutyTicks = (_deadTicks << 1) + INTERRUPT_DELAY;    // Set the minimum number of duty ticks for Timer2 MR1, accouting for the time needed to process the MR0 interrupt
    LPC_PWM1->MR0 = periodTicks;                      // Change MR0 match register value to adjust PWM1 period
    LPC_PWM1->LER = 0x7F;                                   // Set latch-enable register, activates changes to PWM1 on the next MR0 match event (next period of the wave)
}

// Return the current frequency setting
uint32_t MotorDriver::getFreq() {
    return (uint32_t)((float)((PCLK_HZ) / periodTicks));
}

// Change duty cycle by changing PWM1 match registers and updating
// Timer2 register array so they take effect at the same time
void MotorDriver::setDuty(int duty) {
    duty += _minDutyTicks;                          // Allow user input duty to start from 0 by adding offset
    if (duty > _maxDutyTicks)                       // Enforce min and max duty
        duty = _maxDutyTicks;
    if (duty < _minDutyTicks)
        duty = _minDutyTicks;
    
    _dutyTicks = duty;                              // Store for retrieval later

    Timer2MR[_pinPair - 1] = duty;           // Update the Timer2 register array

    // Update PWM1 Match Registers
    if (_pinPair == 1) {
        LPC_PWM1->MR1 = _deadTicks + INTERRUPT_DELAY;       // Set (HIGH) after deadtime, accounting for interrupt delay time
        LPC_PWM1->MR2 = duty - _deadTicks;                  // Clear (LOW) before end of the pulse

    } if (_pinPair == 2) {
        LPC_PWM1->MR3 = _deadTicks + INTERRUPT_DELAY;
        LPC_PWM1->MR4 = duty - _deadTicks;

    } if (_pinPair == 3) {
        LPC_PWM1->MR5 = _deadTicks + INTERRUPT_DELAY;
        LPC_PWM1->MR6 = duty - _deadTicks;
    }
    
    LPC_PWM1->LER = 0x7F;                  // Set latch-enable register to enable the previous writes on next MR0 match
}

// Get the max user input duty - largest number setDuty() will take
uint32_t MotorDriver::getMaxDuty() {
    return (_maxDutyTicks - _minDutyTicks);
}

// Set duty with a float
void MotorDriver::setDuty(float duty) {
    
    // Check limits
    if (duty > 1) duty = 1;
    if (duty < 0) duty = 0;
    
    setDuty((int)((duty * (_maxDutyTicks - _minDutyTicks))));       // Convert to int and send to setDuty()
}

// Return current duty ticks from 0 to max user input ticks
int MotorDriver::readDuty() {
    return (_dutyTicks - _minDutyTicks);
}

// Return current duty ticks as float
float MotorDriver::readDutyF() {
    return (float)((float)(_dutyTicks - _minDutyTicks) / (float)(_maxDutyTicks - _minDutyTicks));
}

// Adjust deadticks
void MotorDriver::setDeadTicks(uint32_t deadTicks) {

    // Set limit if too high
    if (deadTicks > (periodTicks >> 1))
        deadTicks = periodTicks >> 1;

    _deadTicks = deadTicks;

    // Recalculate absolute max and min duty ticks
    _maxDutyTicks = periodTicks;
    _minDutyTicks = (_deadTicks << 1) + INTERRUPT_DELAY;

    setDuty(readDuty());        // Reset duty to refresh with newly computed values based on new deadTicks
}

// Get current deadTicks
uint32_t MotorDriver::getDeadTicks() {
    return _deadTicks;
}

// Disable the current MotorDriver outputs, set to high impedance inputs
void MotorDriver::disable() {
    

    // Disable PWM output, set flag to let interrupt know output is disabled
    isEnabled[_pinPair - 1] = false;
    LPC_PWM1->PCR &= ~(1 << ((2 * _pinPair) + 8));
    
    // Set to the pin configuration register back to GPIO
    LPC_PINCON->PINSEL4 &= ~(1 << ((4 * _pinPair) - 2));        // Configure pin as GPIO
    LPC_PINCON->PINSEL0 &= ~(3 << ((2 * _pinPair) + 12));       // Configure matching timer pin as GPIO

    // Zero the output so its reset on next startup
    LPC_TIM2->MR1 = _minDutyTicks;
    LPC_TIM2->MR2 = _minDutyTicks;
    LPC_TIM2->MR3 = _minDutyTicks;    
    setDuty(0);
    
}

// Enable a MotorDriver pin pair, must do this after constructor to start using the pins
void MotorDriver::enable() {

    // Set the pin configuration register to pwm1 and timer2 match outputs
    LPC_PINCON->PINSEL4 |= (1 << ((4 * _pinPair) - 2));        // Configure pin as pwm1
    LPC_PINCON->PINSEL0 |= (3 << ((2 * _pinPair) + 12));       // Configure matching timer pin
    
    // Enable PWM output and select double edged operation
    LPC_PWM1->PCR |= (1 << ((2 * _pinPair) + 8)) | (1 << (_pinPair * 2));
    
    isEnabled[_pinPair - 1] = true;     // Set enabled flag
}
