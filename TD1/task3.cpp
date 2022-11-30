/*
This code is Task 3 of Technical Demonstration 1. 
Measuring speed and angular velocity via the equation given in handbook. Ticker is called repeatedly to calculate throughout the program.
Written by: Adelene - Group 15
Joystick firebutton to turn off the motors and get the encoder values by manual turning.
X2 encoding, bipolar, and lcd screen is used in this program.
*/
#include "mbed.h" //mbed library
#include "QEI.h" //For encoder
#include "C12832.h" //For lcd library

//Pins starts here
PwmOut M1(PC_8);   //set PC_8 as PWMOut Wheel 1 is left 
PwmOut M2(PC_9); //set PC_9 as PWMOut wheel 2 is right
DigitalOut ena(PA_13); //enable is PA_13
DigitalOut Bp1(PC_0); //PC_0 is connected to bipolar pin
DigitalOut direct1(PC_1); //direction pins PC_1
DigitalOut Bp2(PB_0); //bipolar 2 pins is PB_0
DigitalOut direct2(PA_4); //direction pin PA_4


QEI wheel1(PC_10, PC_12, NC, 512);//QEI (PinName channelA, PinName channelB, PinName index, int pulsesPerRev, Encoding encoding=X2_ENCODING)
QEI wheel2(PC_11, PD_2, NC, 512); //by default -> use X2 encoding unless specified. QEI::X4_ENCODING at the end. So 512.

C12832 lcd(D11, D13, D12, D7, D10); //Created LCD object from LCD library

InterruptIn fireButton(D4);
//Pins ends here

float sampleTime = 0.01;
float pulse1, pulse2, prevPulse1, prevPulse2;
float speed, angularSpeed, w1Speed, w2Speed, operatedTime;
Ticker calcSpeedTicker;

typedef enum {set, forwardState, setStop, stopState} ProgramState;   
ProgramState state = set;     //initial state = 'set'      

//function code starts here
void driveForward(void){
    ena.write(1);
    Bp1.write(1);
    Bp2.write(1);
    M1.write(0.65f); 
    M2.write(0.65f); 
}

void stop(void){
    ena.write(0);
}


//ISR code starts here
void speedCalcISR(){
    float encodeTicksPerSecW1 = (pulse1 - prevPulse1) / sampleTime;
    float encodeTicksPerSecW2 = (pulse2 - prevPulse2) /sampleTime;
    w1Speed = (encodeTicksPerSecW1 * 2 * 3.1412159 * 0.00375 * 0.867) / 512; //0.00867 for m/s, 0.867 so cm/s
    w2Speed = (encodeTicksPerSecW2 * 2 * 3.1412159 * 0.00375 * 0.867) / 512;
    speed = (w1Speed + w2Speed)/2; //translational velocity
    angularSpeed = (w1Speed - w2Speed) / 0.156; //156mm is distance between wheels
}

void forwardFirePressedISR(){
    state = setStop;   
}
void stopFirePressedISR(){
    state = forwardState;
}
void turnAroundSecISR(){
    state = set;   
}
//End of function code

 //main loop code starts here
int main() {
    M1.period(0.00005f);
    M2.period(0.00005f);
    direct1 = 0;
    direct2 = 0;
    while(1){
        switch (state) {                           
            case (set) :     
                wheel1.reset();
                wheel2.reset();
                pulse1 = 0;
                pulse2 = 0;
                lcd.cls();
                calcSpeedTicker.attach(&speedCalcISR, sampleTime);
                state = forwardState;
                break;
            case (forwardState):
                driveForward();
                pulse1 = wheel1.getPulses();
                pulse2 = wheel2.getPulses();
                lcd.locate(0,0);
                lcd.printf("Spd = %0.2f", speed);
                lcd.locate(60,0);
                lcd.printf("Ang Spd = %0.2f", angularSpeed);
                lcd.locate(0,10);
                lcd.printf("P1 = %0.2f", pulse1);
                lcd.locate(0,20);
                lcd.printf("P2 = %0.2f", pulse2);
                prevPulse1 = pulse1;
                prevPulse2 = pulse2;
                fireButton.rise(&forwardFirePressedISR);
                break;
            case (setStop):
                lcd.cls();
                wheel1.reset();
                wheel2.reset();
                state = stopState;
            case (stopState):
                stop();
                pulse1 = wheel1.getPulses();
                pulse2 = wheel2.getPulses();
                lcd.locate(0,0);
                lcd.printf("Sd = %0.2f", speed);
                lcd.locate(60,0);
                lcd.printf("Ang Sd = %0.2f", angularSpeed);
                lcd.locate(0,10);
                lcd.printf("P1 = %0.2f", pulse1);
                lcd.locate(0,20);
                lcd.printf("P2 = %0.2f", pulse2);
                prevPulse1 = pulse1;
                prevPulse2 = pulse2;
                fireButton.rise(&stopFirePressedISR);
                break;
            default :                               // If unexpected State occurs.
                stop();
        }
    }
}
