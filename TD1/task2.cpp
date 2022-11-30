/**
This is Task 2 (Controls of Motor) for Technical Demonstration 1.
Written by: Adelene - Group 15
Using Bipolar Mode
Potentiometer is used to control the speed of the motors.
Since it is in bipolar mode, 50% duty cycle -> stationary!
*/
#include "mbed.h" //mbed library
#include "C12832.h" //For lcd library

//<--- Class Declarations --->
class Potentiometer                                 
{
private:                                         
    AnalogIn inputSignal;                           //Declaration of AnalogIn object
    float VDD, currentSampleNorm, currentSampleVolts; //Float variables to speficy the value of VDD and most recent samples

public:                                           
    Potentiometer(PinName pin, float v) : inputSignal(pin), VDD(v) {}   //Constructor - user provided pin name assigned to AnalogIn...
                                                                        //VDD is also provided to determine maximum measurable voltage
    float amplitudeVolts(void)                      //Public member function to measure the amplitude in volts
    {
        return (inputSignal.read()*VDD);            //Scales the 0.0-1.0 value by VDD to read the input in volts
    }
    
    float amplitudeNorm(void)                       //Public member function to measure the normalised amplitude
    {
        return inputSignal.read();                  //Returns the ADC value normalised to range 0.0 - 1.0
    }
    
    void sample(void)                               //Public member function to sample an analogue voltage
    {
        currentSampleNorm = inputSignal.read();       //Stores the current ADC value to the class's data member for normalised values (0.0 - 1.0)
        currentSampleVolts = currentSampleNorm * VDD; //Converts the normalised value to the equivalent voltage (0.0 - 3.3 V) and stores this information
    }
    
    float getCurrentSampleVolts(void)               //Public member function to return the most recent sample from the potentiometer (in volts)
    {
        return currentSampleVolts;                  //Return the contents of the data member currentSampleVolts
    }
    
    float getCurrentSampleNorm(void)                //Public member function to return the most recent sample from the potentiometer (normalised)
    {
        return currentSampleNorm;                   //Return the contents of the data member currentSampleNorm  
    }

};
//Sampling Potentiometer Class declaration: inheritance
class SamplingPotentiometer : public Potentiometer { //applicable for left and right potentiometer
private:
    float samplingFrequency, samplingPeriod; //variable to hold frequency and period.
    Ticker sampler; //Ticker named Sampler.
    
public:
    SamplingPotentiometer(PinName p, float v, float fs) : Potentiometer(p,v) //v = change? fs = how often to check potentiometer? 5x seconds (immediate)
    {samplingFrequency = fs;
    samplingPeriod = 1.0/samplingFrequency; 
    sampler.attach(callback(this, &SamplingPotentiometer::sample), samplingPeriod);}   //ticker checks the potentiometer every sampling period.
    float value(void)
    {
        float norm = getCurrentSampleNorm();
        return norm;
    }
};
//<----------------END OF CLASS DECLARATIONS----------------->

//Pins starts here
PwmOut M1(PC_8);   //set PC_8 as PWMOut Wheel 1 is left CHANGE - PB_14
PwmOut M2(PC_9); //set PC_9 as PWMOut wheel 2 is right CHANGE - PB13
DigitalOut ena(PA_13); //enable is PA_13
DigitalOut Bp1(PB_3); //PC_0 is connected to bipolar pin
DigitalOut direct1(PA_10); //direction pins PC_1
DigitalOut Bp2(PB_4); //bipolar 2 pins is PB_0
DigitalOut direct2(PB_5); //direction pin PA_4

C12832 lcd(D11, D13, D12, D7, D10); //Created LCD object from LCD library

InterruptIn fireButton(D4);

SamplingPotentiometer right(A1, 1, 5); //right potentiomenter for w1 Duty Cycle
SamplingPotentiometer left(A0, 1, 5); //left potentiometer for w2 Duty Cycle
//Pins ends here

float dutyCycleControlw1, dutyCycleControlw2;

typedef enum {set, forwardState, setStop, stopState, setTurnRight, turnRightState, setTurnAround, turnAroundState, setTurnLeft, turnLeftState} ProgramState;   
ProgramState state = set;     //initial state = 'set'      

//function code starts here
void driveForward(void){
    ena.write(1);
    Bp1.write(1);
    Bp2.write(1);
    M1.write(dutyCycleControlw1); //duty cycle 35%
    M2.write(dutyCycleControlw2); //duty
}
void stop(void){
    ena.write(0); //turn off enable
}

void forwardFirePressedISR(){
    state = setStop;   
}
void stopFirePressedISR(){
    state = forwardState;
}
//End of function code

 //main loop code starts here
int main() {
    M1.period_us(50);
    M2.period_us(50);
    direct1 = 0;
    direct2 = 0;
    while(1){
        switch (state) {                           
            case (set) :     
                lcd.cls();
                state = forwardState;
                break;
            case (forwardState):
                dutyCycleControlw1 = left.value(); //w1 is left
                dutyCycleControlw2 = right.value(); //w2 is right
                lcd.locate(0,0);
                lcd.printf("Moving");
                lcd.locate(0,10);
                lcd.printf("Duty Cycle w1 = %0.2f", dutyCycleControlw1);
                lcd.locate(0,20);
                lcd.printf("Duty Cycle w2 = %0.2f", dutyCycleControlw2);
                driveForward();
                fireButton.rise(&forwardFirePressedISR);
                break;
            case (setStop):
                lcd.cls();
                state = stopState;
            case (stopState):
                dutyCycleControlw1 = left.value(); //w1 is left
                dutyCycleControlw2 = right.value(); //w2 is right
                lcd.locate(0,0);
                lcd.printf("Motor stops");
                lcd.locate(0,10);
                lcd.printf("Duty Cycle w1 = %0.2f", dutyCycleControlw1);
                lcd.locate(0,20);
                lcd.printf("Duty Cycle w2 = %0.2f", dutyCycleControlw2);
                stop();
                fireButton.rise(&stopFirePressedISR);
                break;
            default :                               // If unexpected State occurs.
                stop();
        }
    }
}
