#include "mbed.h"
#include "C12832.h"
/* Set up serial communication between the HM-10 module and the Nucleo board
For debugging and setting up the module, set up serial between Nucleo and PC Set up LED for debugging*/
Serial hm10(PA_11,PA_12); //PA_11 is tx, PA_12 is rx (make sure both of them cross)
Serial pc(USBTX, USBRX);

PwmOut M1(PC_9);   //set PC_8 as PWMOut Wheel 1 is left CHANGE - PB_14
PwmOut M2(PC_8); //set PC_9 as PWMOut wheel 2 is right CHANGE - PB13
DigitalOut ena(PA_13); //enable is PA_13
DigitalOut Bp1(PB_3); //PC_0 is connected to bipolar pin
DigitalOut direct1(PC_14); //direction pins PC_1
DigitalOut Bp2(PB_4); //bipolar 2 pins is PB_0
DigitalOut direct2(PC_0); //direction pin P

C12832 lcd(D11, D13, D12, D7, D10);

void driveForward(void){
    ena.write(1);
    Bp1.write(0);
    Bp2.write(0);
    direct1.write(1); //change 0 to 1
    direct2.write(0);
    M1.write(0.90);
    M2.write(0.90);
}
void stop(void){
    ena.write(0); //turn off enable
}

char s;
char w;
void serial_config();
int main(){
     M1.period_us(50);
     M2.period_us(50);
     lcd.locate(0,10);
     lcd.printf("Value");
     pc.baud(9600);
     hm10.baud(9600);//Set up baud rate for serial communication while (!hm10.writeable()) { } //wait until the HM10 is ready while(1) {
     while(1) {
         driveForward();
         if (s == 's'){
             stop();
         }
         else if (s == 'g'){
             driveForward();
        }
         if (hm10.readable()) {
              s = hm10.getc();
              lcd.locate(0,0);
              lcd.printf("%c", s);
        }
         serial_config();
     }}
/* serial_config allows you to set up your HM-10 module via USB serial port*/ 
void serial_config(){
     if (pc.readable()){
         w = pc.getc();
         hm10.putc(w);
     }
}
