#include "mbed.h"
#include "USBHostMSD.h"
#include "wave_player.h"
//mbed Application board waveplayer demo
//Plays the wave file "sample.wav" on the USB flash drive
//Outputs to onboard speaker (but at very low volume)
//and the Audio Out jack for connection to a set of amplified PC speakers (at higher volume)
//Needs a USB flash drive inserted with the wav file on it to run

//Analog Out Jack
AnalogOut DACout(p18);
//On Board Speaker
PwmOut PWMout(p26);

wave_player waver(&DACout,&PWMout);

int main()
{
    USBHostMSD msd("usb");
    FILE *wave_file;
    //setup PWM hardware for a Class D style audio output
    PWMout.period(1.0/400000.0);
    printf("\n\n\nHello, wave world!\n");
    // wait until connected to a USB device
    while(!msd.connect()) {
        Thread::wait(500);
    }
    //open wav file and play it
    wave_file=fopen("/usb/sample.wav","r");
    waver.play(wave_file);
    fclose(wave_file);
    //end of program
    while(1) {};
}