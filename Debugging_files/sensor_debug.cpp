/*
Written by Adelene 
Changed pins on pcb onto the buggy.
Testing sensor and output on lcd screen
*/
#include "mbed.h"
#include "C12832.h"
class sensor{
private:
    AnalogIn inputs;
    DigitalOut darlington;
    float reading;
public:
    sensor(PinName inputPin,PinName darPin): inputs(inputPin), darlington(darPin) {sensor::clearReading();}
    float getReading(void){
        //reads the digital input pin and return
        reading = inputs.read();
        return reading;
    }
    void clearReading(){
        reading = 0;
    }
    void on(){
        darlington = 1;
    }
    void off(){
        darlington = 0;
    }
    /*
    void sense(){
        if (getReading() > 0.60){ //0.70
            logic = 1;
        }
        else {
            logic = 0;
        } 
    } */
};

//Pin declaration 


sensor s1(PA_0,PC_7); //analog pin, darlington pin
sensor s2(PC_2,PH_1); //2nd PCB S2 is PB_9, 1st buggy PCB S2 is PA_10
sensor s3(PC_3,PA_5); 
sensor s4(PC_5,PH_0);
sensor s5(PB_1,PA_8); 
sensor s6(PC_4,PB_8); 


C12832 lcd(D11, D13, D12, D7, D10);

void onSensors(){
    s1.on();
    s2.on();
    s3.on();
    s4.on();
    s5.on();
    s6.on();
}
void offSensors(){
    s1.off();
    s2.off();
    s3.off();
    s4.off();
    s5.off();
    s6.off();
}

int main(){
    s1.off();
    s2.off();
    s3.off();
    s4.off();
    s5.off();
    s6.on();
    float voltage1, voltage2, voltage3, voltage5, voltage6, voltage4;
    while(1){
        voltage1 = (s1.getReading())*10;
        voltage2 = (s2.getReading())*10;
        voltage3 = (s3.getReading())*10;
        voltage4 = (s4.getReading())*10;
        voltage5 = (s5.getReading())*10;
        voltage6 = (s6.getReading())*10;
        lcd.locate(0,0);
        lcd.printf("V1 = %f", voltage1);
        lcd.locate(0,10);
        lcd.printf("V4 = %f", voltage4);
        lcd.locate(0,20);
        lcd.printf("V6 = %f", voltage6);
    }
}
