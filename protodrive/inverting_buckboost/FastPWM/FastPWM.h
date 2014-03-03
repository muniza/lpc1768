/*
    .---.           _....._
   /  p  `\     .-""`:     :`"-.
   |__   - |  ,'     .     '    ',
    ._>    \ /:      :     ;      :,
     '-.    '\`.     .     :     '  \
        `.   | .'._.' '._.' '._.'.  |
          `;-\.   :     :     '   '/,__,
          .-'`'._ '     .     : _.'.__.'
         ((((-'/ `";--..:..--;"` \
             .'   /           \   \
       jgs  ((((-'           ((((-'
       
Yeah ASCII art turtle more fun than copyright stuff
*/


#include "mbed.h"


#ifndef FASTPWM_H
#define FASTPWM_H

#ifndef F_CLK
#define F_CLK   96000000
#endif

/** Library that allows faster and/or higher resolution PWM output
  *
  * Library can directly replace standard mbed PWM library. Only limitation is that the maximum PWM period is four times shorter
  * The maximum achievable period is roughly 40 seconds, I dont think that should be a problem.
  * Do take into account all PWM objects will run four times faster than default.
  *
  * Contrary to the default mbed library, this library takes doubles instead of floats. The compiler will autocast if needed,
  * but do take into account it is done for a reason, your accuracy will otherwise be limitted by the floating point precision.
  *
  * In your program you can define F_CLK if you use a different clock frequency than the default one. 
  *
  * Only works on LPC1768 for now. If you want support for the other one, send a PM and I will have a look, but I cannot even compile for it.
  */
class FastPWM {
public:
    /**
    * Create a FastPWM object connected to the specified pin
    *
    * @param pin - PWM pin to connect to
    */
    FastPWM(PinName pin);
    
    /**
    * Set the PWM period, specified in seconds (double), keeping the duty cycle the same.
    */
    void period(double seconds);
    
    /**
    * Set the PWM period, specified in milli-seconds (int), keeping the duty cycle the same.
    */
    void period_ms(int ms);
    
    /**
    * Set the PWM period, specified in micro-seconds (int), keeping the duty cycle the same.
    */
    void period_us(int us);
    
    /**
    * Set the PWM period, specified in micro-seconds (double), keeping the duty cycle the same.
    */
    void period_us(double us);
    
    /**
    * Set the PWM pulsewidth, specified in seconds (double), keeping the period the same.
    */
    void pulsewidth(double seconds);
    
    /**
    * Set the PWM pulsewidth, specified in milli-seconds (int), keeping the period the same.
    */
    void pulsewidth_ms(int ms);
    
    /**
    * Set the PWM pulsewidth, specified in micro-seconds (int), keeping the period the same.
    */
    void pulsewidth_us(int us);
    
    /**
    * Set the PWM pulsewidth, specified in micro-seconds (double), keeping the period the same.
    */
    void pulsewidth_us(double us);
    
    /**
    * Set the ouput duty-cycle, specified as a percentage (double)
    *
    * @param duty - A double value representing the output duty-cycle, specified as a percentage.  The value should lie between 0.0 (representing on 0%) and 1.0 (representing on 100%).
    */
    void write(double duty);
    
    /**
    * Return the ouput duty-cycle, specified as a percentage (double)
    *
    * @param return - A double value representing the output duty-cycle, specified as a percentage.
    */
    double read( void );
    
    /**
    * An operator shorthand for write()
    */
    FastPWM& operator= (double value);
    
    /**
    * An operator shorthand for read()
    */
    operator double();
    
private:
    PwmOut PWMObject;
    double _duty;
    double _period;
    unsigned int PWMUnit;
    
    __IO uint32_t *MR;

};
#endif