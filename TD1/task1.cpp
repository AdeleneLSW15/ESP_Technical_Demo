/**
This is Task 1 (PWM Duty Cycle) for Technical Demonstration 1.
Written by: Adelene  - Group 15
Using Bipolar Mode
*/
#include "mbed.h" //mbed library

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

//Pins starts here
PwmOut M1(PC_8);   //set PC_8 as PWMOut Wheel 1 is left CHANGE - PB_14
PwmOut M2(PC_9); //set PC_9 as PWMOut wheel 2 is right CHANGE - PB13
DigitalOut ena(PA_13); //enable is PA_13
DigitalOut Bp1(PC_0); //PC_0 is connected to bipolar pin
DigitalOut direct1(PC_1); //direction pins PC_1
DigitalOut Bp2(PB_0); //bipolar 2 pins is PB_0
DigitalOut direct2(PA_4); //direction pin PA_4
//Pins ends here


SamplingPotentiometer right(A1, 1, 5);

 //main loop code starts here
int main() {
    M1.period(0.00005f);
    M2.period(0.00005f);
    ena = 0; //turn off the motors during assessment.
    Bp1 = 1;
    Bp2 = 1;
    direct1 = 0;
    direct2 = 0;
    while(1){
        float dutyCycle = right.value();
        M1.write(dutyCycle); //60% duty cycle
        M2.write(dutyCycle); //60% duty cycle
    }
}
