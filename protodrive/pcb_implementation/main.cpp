#include "mbed.h"
#include "Controller.h"
#include "BuckBoostDriver.h"

Serial pc(USBTX, USBRX);
bool running = false;           // Is the system clear to run?

// pinName (p25 w/p7, p23 w/p6, p21 w/p5)
MotorDriver driveMotor  (p25, 20000, 50);
MotorDriver loadMotor   (p23, 20000, 50);

const float vSenseMult = 0.00105866;
const float iSenseMult = 0.0000822116;
const int vSenseOffset = -957;
const int iSenseOffset = -30337;
Tachometer tach         (p8, 8, 4, 0.5, FALLING);  // Motor speed tachometer input
IVSensors driveSensors  (0 , 1 , 2 , 4, vSenseMult, vSenseOffset, iSenseMult, iSenseOffset);  // Current and voltage sensor on drive motor
IVSensors loadSensors   (3 , 4 , 5 , 4, vSenseMult, vSenseOffset, iSenseMult, iSenseOffset);  // Current and voltage sensor on load motor
IVSensors driveBattery  (6 , 7 , 8 , 4, vSenseMult, vSenseOffset, iSenseMult, iSenseOffset);  // Current and voltage sensor on drive battery
IVSensors loadBattery   (9 , 10, 11, 4, vSenseMult, vSenseOffset, iSenseMult, iSenseOffset);  // Current and voltage sensor on load battery
//IVSensors driveSC       (12, 13, 14, 1, vSenseMult, 0.0, iSenseMult, 0.0);  // Current and voltage sensor on drive super cap
//IVSensors buckboostSC   (15, -1, -1, 1, vSenseMult, 0.0, iSenseMult, 0.0);  // Current sensor between super cap and DC battery bus

Controller drive        (&driveMotor, &tach, &driveSensors, 1000, 0.01, 0.0, 1.0, 18.0, 5, (18 * 5), 10000.0);
Controller load         (&loadMotor , &tach, &loadSensors , 1000, 0.01, 0.0, 1.0, 18.0, 5, (18 * 5), 10000.0);

DigitalOut led1(LED1);
DigitalOut extra0(p19);
DigitalOut extra1(p18);
DigitalOut extra2(p17);
DigitalOut extra3(p16);
DigitalOut extra4(p15);

#include "SerialCommands.h"

int main() {
    AnalogMux::MuxSetup(p20, p11, p29, p30, p10, p9);
    pc.baud(9600);
    pc.printf("Reset.\r\n");
    unsigned int loopCount = 0;
    wait(0.5);
    while(1) {
        
        // Do much less frequently
        if (loopCount % 16 == 0) {
            loadBattery.update();
            driveBattery.update();
        }
        drive.updateInputs();
        load.updateInputs();

        // Run the control loops if the program is active and receiving external commands
        if (running) {
            drive.output();
            load.output();
            led1=1;
        }
        // Check serial buffers, execute any waiting commands
        serialCheck();
        loopCount++;
    }
}