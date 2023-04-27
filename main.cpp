#include "Mutex.h"
#include "mbed.h"
#include "Motor.h"
#include "speaker.hpp"
#include "rtos.h"

Speaker mySpeaker(p21);

DigitalIn button(p20);

DigitalIn sonarEcho(p7);
DigitalOut sonarTrigger(p6);
Timer sonarTimer;
int sonarDistance = 0;
int sonarCorrection = 0;
Mutex sonar_mutex;

DigitalOut led(LED1);

//PwmOut MotorASpeed(p22);
//DigitalOut MotorAForward(p15);
//DigitalOut MotorAReverse(p16);
Motor motorA(p22,p15,p16);

//PwmOut MotorBSpeed(p23);
//DigitalOut MotorBForward(p17);
//DigitalOut MotorBReverse(p18);
Motor motorB(p23,p18,p17);

Serial esp(p28, p27);
Serial pc(USBTX, USBRX);
DigitalOut wifiReset(p26);
Timer wifiTimer;
int  count,ended,timeout;
char buf[2024];
char snd[1024];
char ssid[32] = "AndroidAP";     // enter WiFi router ssid inside the quotes
char pwd [32] = "password"; // enter WiFi router password inside the quotes


enum alarmState {ALARMING, BUTTONPRESSED, OFF};
alarmState alarm;
Mutex alarming_mutex;

void dev_recv()
{
    while(esp.readable()) {
        pc.putc(esp.getc());
    }
}
 
void pc_recv()
{
    while(pc.readable()) {
        esp.putc(pc.getc());
    }
}

void SendCMD()
{
    esp.printf("%s", snd);
}
 
void getreply()
{
    memset(buf, '\0', sizeof(buf));
    wifiTimer.start();
    ended=0;
    count=0;
    while(!ended) {
        if(esp.readable()) {
            buf[count] = esp.getc();
            count++;
        }
        if(wifiTimer.read() > timeout) {
            ended = 1;
            wifiTimer.stop();
            wifiTimer.reset();
        }
    }
}

void ESPconfig()
{

    wait(5);
    pc.printf("\f---------- Starting ESP Config ----------\r\n\n");
        strcpy(snd,".\r\n.\r\n");
    SendCMD();
        wait(1);
    pc.printf("---------- Reset & get Firmware ----------\r\n");
    strcpy(snd,"node.restart()\r\n");
    SendCMD();
    timeout=5;
    getreply();
    pc.printf(buf);
 
    wait(2);
 
    pc.printf("\n---------- Get Version ----------\r\n");
    strcpy(snd,"print(node.info())\r\n");
    SendCMD();
    timeout=4;
    getreply();
    pc.printf(buf);
 
    wait(3);
 
    // set CWMODE to 1=Station,2=AP,3=BOTH, default mode 1 (Station)
    pc.printf("\n---------- Setting Mode ----------\r\n");
    strcpy(snd, "wifi.setmode(wifi.STATION)\r\n");
    SendCMD();
    timeout=4;
    getreply();
    pc.printf(buf);
 
    wait(2);
 
   
 
    // pc.printf("\n---------- Listing Access Points ----------\r\n");
    // strcpy(snd, "function listap(t)\r\n");
    //     SendCMD();
    //     wait(1);
    //     strcpy(snd, "for k,v in pairs(t) do\r\n");
    //     SendCMD();
    //     wait(1);
    //     strcpy(snd, "print(k..\" : \"..v)\r\n");
    //     SendCMD();
    //     wait(1);
    //     strcpy(snd, "end\r\n");
    //     SendCMD();
    //     wait(1);
    //     strcpy(snd, "end\r\n");
    //     SendCMD();
    //     wait(1);
    //     strcpy(snd, "wifi.sta.getap(listap)\r\n");
    // SendCMD();
    // wait(1);
    //     timeout=15;
    // getreply();
    // pc.printf(buf);
 
    wait(2);
 
    pc.printf("\n---------- Connecting to AP ----------\r\n");
    pc.printf("ssid = %s   pwd = %s\r\n",ssid,pwd);
    strcpy(snd, "wifi.sta.config(\"");
    strcat(snd, ssid);
    strcat(snd, "\",\"");
    strcat(snd, pwd);
    strcat(snd, "\")\r\n");
    SendCMD();
    timeout=10;
    getreply();
    pc.printf(buf);
 
    wait(5);
 
    pc.printf("\n---------- Get IP's ----------\r\n");
    strcpy(snd, "print(wifi.sta.getip())\r\n");
    SendCMD();
    timeout=3;
    getreply();
    pc.printf(buf);
 
    wait(1);
 
    pc.printf("\n---------- Get Connection Status ----------\r\n");
    strcpy(snd, "print(wifi.sta.status())\r\n");
    SendCMD();
    timeout=5;
    getreply();
    pc.printf(buf);
 
    pc.printf("\n\n\n  If you get a valid (non zero) IP, ESP8266 has been set up.\r\n");
    pc.printf("  Run this if you want to reconfig the ESP8266 at any time.\r\n");
    pc.printf("  It saves the SSID and password settings internally\r\n");
    wait(5);
}

void wifiFunc(void const *args) {
    // Init wifi
    wifiReset=0; //hardware reset for 8266
    pc.baud(9600);
    wifiReset = 1;
    timeout=2;
    getreply();
    esp.baud(9600);
    ESPconfig();
    pc.attach(&pc_recv, Serial::RxIrq);
    esp.attach(&dev_recv, Serial::RxIrq);

    // loop
}

void heartbeat(void const *args)
{
    led = 1;
    Thread::wait(1000);
    led = 0;
    Thread::wait(1000);
    // Set up watchdog here?
}

void sonarReadFunc(void const *args) {
    // Init sonar
    sonarTimer.reset();
    sonarTimer.start();
    while (sonarEcho==2) {};
    sonarTimer.stop();
    sonarCorrection = sonarTimer.read_us();

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

void buttonFunc(void const *args) {

}

// m.speed(x) where -1.0 <= x <= 1.0
// 0.5 speed for 1 second is about 180 deg
void motorFunc(void const *args) {
    /*
    while (alarm = ALARMING) {
        float speedA = 1;
        float speedB = 1;
        if (sonarDistance <= 8) {
            // Turn
            motorA.speed(0.5);
            motorB.speed(-0.5);
            Thread::wait(1000);
        } else {

        }
    }*/
}

void speakerFunc(void const *args) {
    while(1) {
        if (alarm == ALARMING) {
            mySpeaker.PlayNote(969.0, 0.5, 0.01);
            mySpeaker.PlayNote(800.0, 0.5, 0.01);
        }
    }
}

int main()
{
    alarm = OFF;

    Thread wifi_thread(wifiFunc);
    Thread heartbeat_thread(heartbeat);
    Thread sonar_thread(sonarReadFunc);
    Thread button_thread(buttonFunc);
    Thread motor_thread(motorFunc);
    Thread speaker_thread(speakerFunc);

    while(1) {
        Thread::wait(1000);
    }
}
