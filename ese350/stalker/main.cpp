/*
* Neato 2D Object Tracker Code
* Last edited: 4/26/2013
* Vijay Govindarajan and Alfredo Muniz
* Description: This code allows the Neato to track objects and run into them.
* See GoalieMode.cpp for Additional Comments
*/

#include "mbed.h"
#include <string>

#define MODSERIAL_DEFAULT_RX_BUFFER_SIZE 1024   //One of these is not needed I think
#define MODSERIAL_DEFAULT_TX_BUFFER_SIZE 1024
#include "MODSERIAL.h"  //Normal Serial is limited to 16 byte buffers

MODSERIAL lidar(p9, p10);  // tx, rx
MODSERIAL pc(USBTX, USBRX); // tx, rx

/******************* Neato Communication *********************/
char c;
char s[5000];
int on;
int count;
int num1;
int num2;
int num3;
int num4;
int x1[359];
int y1[359];
int j;
int xball;
int yball;
int xsum;
int ysum;
int points;
int wallL;
int wallSum;
int wallR;
int ballFound;
int xtarget;
int ytarget;

DigitalOut led1(LED1);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

// Timer
Timer timer;

void getldsscan(void)
{
    /******** LIDAR DATA MANIPULATION ********/
    lidar.printf("getldsscan\n");
    for (int i = 0; i < 363; i++) {
        lidar.scanf("%[^\n]%*c",s);
        count = sscanf(s,"%d,%d,%d,%d", &num1, &num2, &num3, &num4);
        if(num2 >= 2000) {
            num2 = 0;
        }
        if(num1 >= 0 && j == 10 && i >= 4) {
            x1[num1] = num2*cos((double)num1*3.14159/180+3.14159/2);
            y1[num1] = num2*sin((double)num1*3.14159/180+3.14159/2);
        }
    }
    j++;
    if (j >= 10) {
        led1 = 1;
    }
    led4 = !led4;

}

int main (void)
{
    lidar.baud(115200);
    pc.baud(115200);

    on = 1;
    j = 0;
    points = 0;
    xsum = 0;
    ysum = 0;
    xtarget = 0;
    ytarget = 0;
    xball = 0;
    yball = 0;
    ballFound = 0;

    wait(1);
    lidar.printf("testmode on\n");
    pc.printf("testmode on\r\n");
    wait(1);
    lidar.printf("setldsrotation on\n");
    pc.printf("setldsrotation on\r\n");

    //Start the timer
    timer.start();


    while(1) {
        if(on) {
            if(timer.read_ms() >= 500) {
                timer.reset();
                getldsscan();
                //Give the lidar some time to warm up before using data
                if(j==11) {
                    //Find the walls so you don't crash into them
                    for (int i = 85; i<95; i++) {
                        wallSum = wallSum + x1[i];
                    }
                    wallL = wallSum/10;
                    pc.printf("wallL = %d\r\n", wallL);
                    wallSum = 0;

                    for (int i = 265; i<275; i++) {
                        wallSum = wallSum + x1[i];
                    }
                    wallR = wallSum/10;
                    pc.printf("wallR = %d\r\n", wallR);
                    wallSum = 0;
                    //Scan the range from 270 to 90 or top half of the robot for obstacles
                    for (int i=275; i<355; i++) {
                        if (x1[i]<(wallR-150) &&
                                x1[i]>(wallL+150) &&
                                abs(x1[i+1]-x1[i])>15 &&
                                abs(x1[i+1]-x1[i])<100 &&
                                x1[i]!=0) {
                            pc.printf("BALL1: %d %d\r\n", x1[i], y1[i]);
                            xsum = xsum + x1[i];
                            ysum = ysum + y1[i];
                            points++;

                        }
                    }

                    for (int i=0; i<86; i++) {
                        if (x1[i]<(wallR-150) &&
                                x1[i]>(wallL+150) &&
                                abs(x1[i+1]-x1[i])>15 &&
                                abs(x1[i+1]-x1[i])<100 &&
                                x1[i]!=0) {
                            pc.printf("BALL1: %d %d\r\n", x1[i], y1[i]);
                            xsum = xsum + x1[i];
                            ysum = ysum + y1[i];
                            points++;

                        }
                    }

                    xball = xsum/points;
                    yball = ysum/points;
                    pc.printf("Ball1: %d %d\r\n", xball, yball);
                    xsum = 0;
                    ysum = 0;
                    points = 0;
                    //If there is a change in x or y then a ball is found else its an error
                    if (xball != 0 && yball != 0) {
                        ballFound = 1;
                    } else {
                        ballFound = 0;
                    }

                    //Move to the target location, notice that -200 is to account for length of robot
                    xtarget = xball;
                    ytarget = yball - 200;
                    //If the obstacle is too close a wall then don't move
                    if(xtarget < (wallL+150) || xtarget > (wallR + 150)) {

                        xtarget = 0;
                        ballFound = 0;
                        pc.printf("Near a wall.");
                    }
                    //If ball object is too close (touching the robot) then don't move
                    if(ytarget < 200) {
                        ytarget = 0;
                        ballFound = 0;
                        pc.printf("Object too close.");
                    }
                    
                    if(ballFound == 1) {
                        //Send the motor command to the robot to move to the target in the y direction
                        lidar.printf("setMotor %d %d %d\n", (int)ytarget, (int)(ytarget*4/3), 250);
                        //Wait until still to start the lidar scan again
                        wait(ytarget/200);
                        //If xtarget is positive then 90 degree spin to the right
                        if (xtarget>0) {
                            lidar.printf("setMotor 140 -186 300\n");
                            wait(1.25);
                            //Move to the target in the x direction
                            lidar.printf("setMotor %d %d %d\n", (int)abs(xtarget)-200, (int)((abs(xtarget)-200)*4/3), 250);
                            wait(abs(xtarget/200));
                            //Spin back to start scanning again
                            lidar.printf("setMotor -140 186 300\n");
                        //Else the robot is to the left so 90 degree left spin
                        } else {
                            lidar.printf("setMotor -140 186 300\n");
                            wait(1.25);
                            lidar.printf("setMotor %d %d %d\n", (int)abs(xtarget)-200, (int)((abs(xtarget)-200)*4/3), 250);
                            wait(abs(xtarget/200));
                            lidar.printf("setMotor 140 -186 300\n");
                        }
                        timer.reset();
                    } else {
                        j = 10;
                    }
                    //Set the counter back to 5 so that there is time to remove obstacle or move person
                    j = 5;
                    led1 = 0;
                    ballFound = 0;
                }
            }
        }
    }
}

/****** Data to Interpret ******
AngleInDegrees,DistInMM,Intensity,ErrorCodeHEX
0,221,1400,0
1,223,1396,0
2,228,1273,0
(. . .)
359,220,1421,0
*/