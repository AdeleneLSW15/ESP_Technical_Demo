/**
This is Task 6 (Square) for Technical Demonstration 1.
Written by: Adelene - Group 15
Using X2 Encoding, Bipolar Mode
A square is drawn out in the process. The final result has offset due to open-loop control (PID will be implemented in TD3).
*/
#include "mbed.h" //mbed library
#include "QEI.h" //For encoder
#include "C12832.h" //For lcd library

//Pins starts here
PwmOut M1(PC_8);   //set PC_8 as PWMOut Wheel 1 is left CHANGE - PB_14
PwmOut M2(PC_9); //set PC_9 as PWMOut wheel 2 is right CHANGE - PB13
DigitalOut ena(PA_13); //enable is PA_13
DigitalOut Bp1(PC_0); //PC_0 is connected to bipolar pin
DigitalOut direct1(PC_1); //direction pins PC_1
DigitalOut Bp2(PB_0); //bipolar 2 pins is PB_0
DigitalOut direct2(PA_4); //direction pin PA_4

QEI wheel1(PC_2, PC_3, NC, 512);//QEI (PinName channelA, PinName channelB, PinName index, int pulsesPerRev, Encoding encoding=X2_ENCODING)
QEI wheel2(PC_4, PC_5, NC, 512); //by default -> use X2 encoding unless specified. QEI::X4_ENCODING at the end. So 512.

C12832 lcd(D11, D13, D12, D7, D10); //Created LCD object from LCD library

InterruptIn fireButton(D4);
//Pins ends here

char flag = 1, turnedArounded = 0;
float sampleTime = 0.01;
int revolutionW1, revolutionW2, countTurn = 0, countTurnL = 0;
float pulse1, pulse2, prevPulse1, prevPulse2;
float speed, angularSpeed, w1Speed, w2Speed, operatedTime;
float dutyCycleControlw1, dutyCycleControlw2;
Timer timeBug;
Ticker calcSpeedTicker;
Timeout turn90left;
Timeout turn90;
Timeout turnAroundSec;
Timeout forwardSec;

typedef enum {set, forwardState, setStop, stopState, setTurnRight, turnRightState, setTurnAround, turnAroundState, setTurnLeft, turnLeftState} ProgramState;   
ProgramState state = set;     //initial state = 'set'      

//function code starts here
void driveForward(void){
    ena.write(1);
    Bp1.write(1);
    Bp2.write(1);
    M1.write(0.65f); //duty cycle 35%
    M2.write(0.65f); //duty
}

void turnRight(void){
    ena.write(1);
    Bp1.write(1);
    Bp2.write(1);
    M1.write(0.65f); //2% duty cycle forward on motor 1
    M2.write(0.50f); //50% duty cycle on motor2 
}

void turnLeft(void){
    ena.write(1);
    Bp1.write(1);
    Bp2.write(1);
    M1.write(0.50f); //50% duty cycle on motor2 
    M2.write(0.65f); //2% duty cycle forward on motor 1
}


void stop(void){
    //ena.write(0); //turn off enable
    M1.write(0.50f); //duty cycle is 50%
    M2.write(0.50f);
}
void turnAround(void){ //1.30 second
    //ena.write(1);
    Bp1.write(1);
    Bp2.write(1);
    //lowers speed when turning around, both wheels move in opposite direction with each other.
    M1.write(0.60f);
    M2.write(0.40f);
}

//ISR code starts here
void speedCalcISR(){
    float encodeTicksPerSecW1 = (pulse1 - prevPulse1) / sampleTime;
    float encodeTicksPerSecW2 = (pulse2 - prevPulse2) /sampleTime;
    w1Speed = (encodeTicksPerSecW1 * 2 * 3.1412159 * 0.00375 * 0.00867) / 512; //0.00867
    w2Speed = (encodeTicksPerSecW2 * 2 * 3.1412159 * 0.00375 * 0.00867) / 512;
    speed = (w1Speed + w2Speed)/2; //translational velocity
    angularSpeed = (w1Speed - w2Speed) / 0.156; //156mm is distance between wheels
}

void forwardSecISR(){
    if (countTurn == 3){
        state = setTurnAround;
        countTurn = 0;
    }
    else {
        if (turnedArounded == 1){
            state = setTurnLeft;
        }
        else {state = setTurnRight;} 
    }
}

void turn90ISR(){
    state = set;
}
void turn90leftISR(){
    if (countTurnL == 4){
        state = setStop;
    }
    else {
        state = set;
    }
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

//Try again but with another method (velocity via encoder ticks per second


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
                turn90.detach();
                turnAroundSec.detach();
                turn90left.detach();
                wheel1.reset();
                wheel2.reset();
                pulse1 = 0;
                pulse2 = 0;
                lcd.cls();
                calcSpeedTicker.attach(&speedCalcISR, sampleTime);
                float forwardTime;
                if (flag == 1){
                    forwardTime = 0.8;
                    flag = 0;
                }
                else {
                    forwardTime = 0.60;
                }
                forwardSec.attach(&forwardSecISR, forwardTime);
                state = forwardState;
                break;
            case (forwardState):
                driveForward();
                pulse1 = wheel1.getPulses();
                pulse2 = wheel2.getPulses();
                lcd.locate(0,0);
                lcd.printf("Speed = %0.2f", speed);
                lcd.locate(0,10);
                lcd.printf("P1 = %0.2f", pulse1);
                lcd.locate(0,20);
                lcd.printf("P2 = %0.2f", pulse2);
                float dutyCycleW1 = M1.read();
                lcd.locate(65,0);
                lcd.printf("DC = %0.2f", dutyCycleW1*100);
                prevPulse1 = pulse1;
                prevPulse2 = pulse2;
                /*
                 if (pulse1 >= 5000 && pulse2 >= 5000){ //21 revolutions, 512 is 1 rev, so 512 *21 is 10752
                    //set a TimeoutISR to calculate time elapsed, once it hits it should change state to forward. unless it is in turnAround state
                    state = setTurnRight;
                    } */
                fireButton.rise(&forwardFirePressedISR);
                break;
            case (setStop):
                turn90.detach();
                turnAroundSec.detach();
                turn90left.detach();
                forwardSec.detach();
                lcd.cls();
                state = stopState;
            case (stopState):
                stop();
                lcd.locate(0,0);
                lcd.printf("Motor stops");
                lcd.locate(0,10);
                dutyCycleW1 = M1.read();
                lcd.printf("DC = %0.2f", dutyCycleW1*100);
                fireButton.rise(&stopFirePressedISR);
                break;
            case (setTurnRight):
                lcd.cls();
                turnAroundSec.detach();
                turn90left.detach();
                forwardSec.detach();
                countTurn += 1; //increments countTurn
                turn90.attach(&turn90ISR, 0.765);
                state = turnRightState;
                break;
            case (turnRightState):
                turnRight();
                pulse1 = wheel1.getPulses();
                pulse2 = wheel2.getPulses();
                lcd.locate(0,0);
                lcd.printf("Turning Right");
                prevPulse1 = pulse1;
                prevPulse2 = pulse2;
                lcd.locate(0,10);
                lcd.printf("Angular Velocity = %0.2f", angularSpeed);
                break;
            case (setTurnLeft):
                forwardSec.detach();
                lcd.cls();
                turn90.detach();
                turnAroundSec.detach();
                turn90left.attach(&turn90leftISR, 0.745);
                countTurnL += 1; //increments countTurn
                state = turnLeftState;
                break;
            case (turnLeftState):
                turnLeft();
                pulse1 = wheel1.getPulses();
                pulse2 = wheel2.getPulses();
                lcd.locate(0,0);
                lcd.printf("Turning Left");
                prevPulse1 = pulse1;
                prevPulse2 = pulse2;
                lcd.locate(0,10);
                lcd.printf("Angular Velocity = %0.2f", angularSpeed);
                break;
            case (setTurnAround):
                lcd.cls();
                for (int i = 0; i < 3; i = i + 1) {
                    stop();
                }
                turnAroundSec.attach(&turnAroundSecISR, 1.22);
                turnedArounded = 1;
                state = turnAroundState;
                break;
            case (turnAroundState):
                lcd.locate(0,0);
                lcd.printf("Turning Around");
                turnAround();
                break;
            default :                               // If unexpected State occurs.
                stop();
        }
    }
}
