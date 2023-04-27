#include "Mutex.h"
#include "mbed.h"
#include "Motor.h"
#include "rtos.h"

PwmOut speaker(p24);

DigitalIn button(p20);

DigitalIn sonarEcho(p7);
DigitalOut sonarTrigger(p6);
Timer sonarTimer;
int sonarDistance = 0;
int sonarCorrection = 0;
Mutex sonar_mutex;

DigitalOut led(LED1);

PwmOut MotorASpeed(p22);
DigitalOut MotorAForward(p15);
DigitalOut MotorAReverse(p16);

PwmOut MotorBSpeed(p23);
DigitalOut MotorBForward(p17);
DigitalOut MotorBReverse(p18);


//Use a PWM output to enable dimming
//1.0 is full on, 0.0 off, 0.5 50% on

void heartbeat(void const *args)
{
    led = 1;
    Thread::wait(1000);
    led = 0;
    Thread::wait(1000);
}

void sonarReadFunc(void const *args) {
    while(1) {
        sonarTrigger = 1;
        sonarTimer.reset();
        Thread::wait(0.01);
        sonarTrigger = 0;
        while (sonarEcho==0) {};
        sonarTimer.start();
        while (sonarEcho==1) {};
        sonarTimer.stop();
        sonarDistance = (sonarTimer.read_us()-sonarCorrection)/58.0;
        Thread::wait(0.2);
    }
}



int main()
{
    sonarTimer.reset();
    sonarTimer.start();
    while (sonarEcho==2) {};
    sonarTimer.stop();
    sonarCorrection = sonarTimer.read_us();




    Thread heartbeat_thread(heartbeat);
    Thread sonar_thread(sonarReadFunc);


    while(1) {
        Thread::wait(1000);
    }

}
