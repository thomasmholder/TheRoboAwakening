#include "mbed.h"
#include "USBHostMSD.h"

DigitalOut led(LED1);

void msd_task(void const *) {
    
    USBHostMSD msd("usb");
    int i = 0;
    
    while(1) {
        
        // try to connect a USB flash disk
        while(!msd.connect())
            Thread::wait(500);
        
        FILE * fp = fopen("/usb/test.txt", "a");
        
        if (fp != NULL) {
            fprintf(fp, "Hello fun SD Card World: %d!\r\n", i++);
            printf("Goodbye World!\r\n");
            fclose(fp);
        } else {
            printf("FILE == NULL\r\n");
        }
        
        // wait until the msd disk is disconnected
        while(msd.connected())
            Thread::wait(500);
    }
}


int main() {
    Thread msdTask(msd_task, NULL, osPriorityNormal, (1024+512) * 4);
    while(1) {
        led=!led;
        Thread::wait(500);
    }
}