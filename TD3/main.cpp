/*
Unipolar Mode
Measuring speed and angular velocity via the equation given in handbook. Ticker is called repeatedly to calculate throughout the program.
Written by: Adelene
X2 encoding, bipolar, and lcd screen is used in this program.
Bluetooth: "s" = stop, "t" = turn, and press any other character other than "b" to move track line position again.
P -> Motors 
Haven't included PD for motors and line position (need for TD4).
*/
#include "mbed.h" //mbed library
#include "C12832.h" //For lcd library
//include other files in here.
#include "motors.cpp"
#include "encoder.cpp"
#include "sensors.cpp"

motor m1(PC_8,PB_4,PB_13); //m1 is left, m2 is right wheel. PH_0
motor m2(PC_9,PB_3,PB_14);
DigitalOut ena(PA_13); //enable is PA_13

encoder wheel1(PC_10,PC_12);
encoder wheel2(PC_11,PD_2);

sensor s1(PC_0,PC_7); //analog pin, darlington pin from PA_0 to PA_1
sensor s2(PC_2,PA_9);
sensor s3(PC_3,PA_5); 
sensor s4(PC_5,PA_6);
sensor s5(PB_1,PA_8); 
sensor s6(PC_4,PB_8);

C12832 lcd(D11, D13, D12, D7, D10); //Created LCD object from LCD library
InterruptIn fireButton(D4);

Serial hm10(PA_11,PA_12); //PA_11 is tx, PA_12 is rx (make sure both of them cross)
Serial pc(USBTX, USBRX);

Timeout endTurn;

//Pins ends here

//Equation to calculate speed and apply to motors
float dutyCycleControlw1 = 1.0, dutyCycleControlw2 = 1.0;
float speed, angularSpeed, w1Speed, w2Speed;
float kp1, kp2, dt;
float lasttime = 0;
float lasterror = 0;
int errorPos;
int SensorValues[6] = {};
char flag = 0;

typedef enum {set, forwardState, setStop, stopState, setTurnAround, turnAround} ProgramState;   
ProgramState state = setStop;     //initial state = 'set'      

//calculation function
float calcDesired(float kp, float kd, float desired, float actualSpeed){
    float currenttime = 0.01;
    dt = currenttime - lasttime;
    float bError = desired - actualSpeed;
    float derror = (bError - lasterror) / dt;
        
    float output = (kp * bError);// + (kd * derror);    // Calculation output for the PD
    /*
    if error is -ve (means actual higher than desired), will make output be a negative output (reduce speed to reach desired)
    if error is +ve (means actual lower than desired), will make output be positive output (increase speed to reach desired)
    */
        
    lasterror = bError;
    lasttime = currenttime;
    return output;
    
}
float setSpeed(float wSpeed, float desired, float kp){
    //kp = 0.001, kd = 0.001, desired is set by user, actual is w1speed
    float dutyControl = calcDesired(kp, 0.05, desired, wSpeed);
    if (dutyControl>= 1.00){
        dutyControl = 0.99;
    }  
    else if(dutyControl <= 0){
        dutyControl = 0.10;
    }
    return dutyControl;
}

//ISR code starts here

void forwardFirePressedISR(){
    state = setStop;   
}
void stopFirePressedISR(){
    state = forwardState;
}
void turnAroundSecISR(){
    state = set;
}

//direction code starts here
//<----function code starts here---->
void driveForward(float idealSpdw1, float idealSpdw2){
    //Kp1 = 0.056, kp2 = 0.057 //kp = 0.070
    dutyCycleControlw1 = setSpeed(w1Speed, idealSpdw1, 0.075); //kp = 0.056
    dutyCycleControlw2 = setSpeed(w2Speed, idealSpdw2, 0.075); //kp = 0.056
    ena.write(1);
    m1.forward(dutyCycleControlw1);
    m2.forward(dutyCycleControlw2); 
}
void stop(void){
    m1.forward(0.00);
    m2.forward(0.00);
    ena.write(0);
}
void turn(float idealSpdw1, float idealSpdw2){
    dutyCycleControlw1 = setSpeed(w1Speed, idealSpdw1, 0.075);
    dutyCycleControlw2 = setSpeed(w2Speed, idealSpdw2, 0.075); 
    ena.write(1);
    m1.forward(dutyCycleControlw1);
    m2.backward(dutyCycleControlw2);
}

//Sensor Function Code starts here
void sensorsOn(){
    s1.on();
    s2.on();
    s3.on();
    s4.on();
    s5.on();
    s6.on();   
}

void sensorArray(){
  sensorsOn();
  SensorValues[0] = s1.digitalize();
  SensorValues[1] = s2.digitalize();
  SensorValues[2] = s3.digitalize();
  SensorValues[3] = s4.digitalize();
  SensorValues[4] = s5.digitalize();
  SensorValues[5] = s6.digitalize();
}

void trackPosition(){
    if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 1 && SensorValues[3] == 1 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //ideal position -> straight line.
        errorPos = 0;
        driveForward(8,8);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 1 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //deviate slightly to the left, up left wheel speed
        errorPos = 1;
        driveForward(10,8);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 1 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //deviate slightly to the right, up right speed.
        errorPos = -1;
        driveForward(8,10);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 1 && SensorValues[2] == 1 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //deviate slightly to the left, correct it by speeding up left wheel (shift right)
        //m1 is left, m2 is right. driveforward(m1,m2)
        errorPos = 1;
        driveForward(10,8);
    }
     else if (SensorValues[0] == 0 && SensorValues[1] == 1 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //deviate slightly to the left, correct it by speeding up left wheel (shift right)
        //m1 is left, m2 is right. driveforward(m1,m2)
        errorPos = 1;
        driveForward(12,8);
    }
    else if (SensorValues[0] == 1 && SensorValues[1] == 1 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //deviate left, correct it by speeding more left wheel (shift right)   
        errorPos = 2;
        driveForward(12,8);
    }
    else if (SensorValues[0] == 1 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //major deviate left, correct it by speeding left wheel significantly (shift right)
        errorPos = 3;
        driveForward(15,4);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 1 && SensorValues[4] == 1 && SensorValues[5] == 0){
        //deviate slightly to the right, correct it by speeding up right wheel (shift left)
        errorPos = -1;
        driveForward(8,10);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 1 && SensorValues[5] == 0){
        //deviate to the right, correct it by speeding up right wheel (shift left)   
        errorPos = -1;
        driveForward(8,12);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 1 && SensorValues[5] == 1){
        //deviate to the right, correct it by speeding up right wheel (shift left)   
        errorPos = -2; 
        driveForward(8,12);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 1){
        //major deviate to the right, correct it by speeding up right wheel significantly (shift left)
        errorPos = -3;
        driveForward(4,15);
    }
    else if (SensorValues[0] == 0 && SensorValues[1] == 0 && SensorValues[2] == 0 && SensorValues[3] == 0 && SensorValues[4] == 0 && SensorValues[5] == 0){
        //no white line detected. Stop the buggy
        stop();
    }
    else{
        //move buggy forward
        driveForward(10,10);
    }
}

void endTurnISR(){
    flag = 0;
}


 //main loop code starts here
char s;
char w;
void serial_config();
int main(){
     flag = 1;
     lcd.locate(0,10);
     lcd.printf("Value");
     pc.baud(9600);
     hm10.baud(9600);//Set up baud rate for serial communication while (!hm10.writeable()) { } //wait until the HM10 is ready while(1) {
     while(1) {
        sensorArray();
        
        w1Speed = wheel1.getSpeed();
        w2Speed = wheel2.getSpeed();
         if (s == 's'){
             stop();
         }
         else if (s == 't'){
             endTurn.attach(&endTurnISR, 0.60);
            s = 'b';
        }
        else if (s == 'b'){
            if (flag == 1){
                     turn(4,7);
                }
            else if (flag == 0){
                    trackPosition();
                }
        }
        else {trackPosition();}
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
