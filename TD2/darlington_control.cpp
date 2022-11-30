/*
The program for Technical Demonstration 2 - Task 4 (Microcontroller).
Written by Adelene 
Use for TD2!
*/
#include "mbed.h"
#include "C12832.h"
class sensor{
private:
    AnalogIn inputs;
    DigitalOut logic;    
    float reading;
public:
    sensor(PinName inputPin, PinName logicPin): inputs(inputPin), logic(logicPin){sensor::clearReading();}
    float getReading(void){
        //reads the digital input pin and return
        reading = inputs.read();
        return reading;
    }
    void clearReading(){
        reading = 0;
    }
    void sense(){
        if (getReading() > 0.60){ //0.70
            logic = 1;
        }
        else {
            logic = 0;
        } 
    }
};

//Pin declaration
DigitalOut darlington1(PB_2);
DigitalOut darlington2(PC_6); //PB_1
DigitalOut darlington3(PB_15);
DigitalOut darlington4(PB_14);
DigitalOut darlington5(PB_13);
DigitalOut darlington6(PB_8); //PC_4
/*
DigitalOut darlington1(PC_9); //originally PC_9 but PWM!
DigitalOut darlington2(PB_8);
DigitalOut darlington3(PB_9);
DigitalOut darlington4(PA_5);
DigitalOut darlington5(PA_6);
DigitalOut darlington6(PD_2);

*/

//darlington ends
/*
AnalogIn sensor1(PA_0);
AnalogIn sensor2(PA_1);
AnalogIn sensor3(PA_4);
AnalogIn sensor4(PB_0);
AnalogIn sensor5(PC_1);
AnalogIn sensor6(PC_0);

DigitalOut led1(PB_4);
DigitalOut led2(PB_5);
DigitalOut led3
*/

sensor s1(PA_0,PB_4); //PA_0
sensor s2(PC_2,PB_5); //PA_1
sensor s3(PC_3,PB_3); //PA_4
sensor s4(PC_5,PA_10); //PB_0
sensor s5(PB_1,PB_9); //PC_1 , PA_2 PB_9
sensor s6(PC_4,PB_12); //PC_0, PA_3 to PB_12


C12832 lcd(D11, D13, D12, D7, D10);

int main(){
    darlington1 = 1;
    darlington2 = 1;
    darlington3 = 1;
    darlington4 = 1;
    darlington5 = 1;
    darlington6 = 1;
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
        lcd.printf("V6 = %f", voltage5);
        s1.sense();
        s2.sense();
        s3.sense();
        s4.sense();
        s5.sense();
        s6.sense(); 
    }
}
