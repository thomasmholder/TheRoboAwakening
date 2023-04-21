#include "Mutex.h"
#include "mbed.h"
#include "Motor.h"
#include "rtos.h"

PwmOut speaker(p24);

DigitalIn button(p20);

DigitalIn sonarEcho(p7);
DigitalOut sonarTrigger(p6);

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




int main()
{

    Thread heartbeat_thread(heartbeat);


    while(1) {
        Thread::wait(1000);
    }

}
