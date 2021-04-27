

// Filename: CMCnibp_V02_2018, code rewritten by Naveen Gangadharan for promini. OLED, serial communication, dated 22-07-2018
/**************************************************************************************************************
* Arduino program for the Arduino ProMini. 
* The program controls two pneumatic pumps and one pneumatic valve; it reads ADC values from a pressure sensor(MP3V5050GP- 0-50kPa).
* There are two switches: (1) START/PAUSE switch, (2) STOP switch . The control has also been enabled from PC using a user interface for changing the pressure steps, pressure values, waiting time and holding time. 
* The upload button loads the values to pressure values, while the start button will start the NIBP machine.
* There are two LEDs, Red (standard) for power supply indication once connected to the supply mains, Red FTDI indicating serial ON. In addition, there are power indicator LED on the ProMini.
* THE OLED displays the set pressure, the current achieved pressure dynamically.
* Function: The program controls pumping of a cuff to a series of pressure values.
* At each pressure value, the cuff pressure is held for the preset seconds, during which time,
* the finger PPG from the occluded arm and the other arm are recorded in CMCdaq; the cuff pressure is also recorded.  
* 
* Notes:
* 1. MachineState=1.
* 1.1. ExperimentState=1, Experiment is running 
* 1.1.1. ExperimentState=1, PumpState=0; Pressure is being held, Pump is OFF and measurements being taken   
* 1.1.2. ExperimentState=1, PumpState=1; Pressure is being changed, Pump is ON and measurements are not be taken 
* 
* 1.2. ExperimentState=0, Experiment is paused. PumpState=0. 
* 
* 2. MachineState=0. 
* 2.1. MachineState=0, All parameters set to initial values. Pneumatic release valve is open.
* 
* 
* Pressure sensor: Pressure= ((analogRead(Pressure_ADCport))*0.4098-7);
* //Note: 50kPa/3.3V => 50kPa/1024 levels => 375mmHg/1024levels => 0.36 mmHg/level ; (Conversion: 1 kPa=7.5 mmHg). The pressures has been calibrated against a sphigmomanometer and a multiplication factor
* has been included, making the eqn as ADC*0.4098-7. For the first pressure the OLED display code has been been written with the -ve offset being considered which makes the pressure eqn (0.4098-7+15).

* switchA toggles ExperimentState when MachineState=1; If MachineState==0) switchA sets MachineState=1, and ExperimentState=1
* switchB always makes MachineState=0; Also sets ExperimentState=0
* 
****************************************************************************************************************/


/***************************************************************************************************************/
/** The following section has been added for OLED**/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);//added oled
/**************************************************/

#define PneumRelease  0
#define PneumPumping  1
#define PneumHolding  2

#define pwrPump1            2        // Digital I/O: Pump MOSFET
#define pwrPump0            3        // Digital I/O: Pump MOSFET
#define pwrValve1           4        // Digital I/O: Release Valve#1 MOSFET
#define pwrValve0           5        // Digital I/O: Valve#0 MOSFET


#define switchA       12       // Digital I/O: 8= switch Pause/Run
#define switchB       13       // Digital I/O: 9= switch Reset to initial values

#define Pressure_ADCport    A0    // A/D channel input: for the Pressure signal

/*****************************************************************************************************************/
/*OLED display function variables used*/
int a, x, x1,x11,x2,x22,x3,x33,p, p1,p11,p2,p22,p3,p33; 
/*****************************************************************************************************************/
const unsigned long maxpumpTime=30000;
/*****************************************************************************************************************/                                                           
unsigned int CuffPressure_mmHg=0;
unsigned long PhaseStartTime[3];
boolean MachineState=0;
boolean swAstate=1, swBstate=1;
boolean ledGToggle, ledBToggle;
unsigned int ExptPhase=0;      // 0=pumping, 1=holding, 2=return to zero Pressure and wait
unsigned long ledGflicktime=0, ledBflicktime=0;
unsigned long swAtime=0, swBtime=0;
unsigned long curTime;
unsigned long PumpONtime=0;

/*for serial buffer data storage*/
unsigned int buffers[27]={};  
byte  buff[1]={};
unsigned int i = 0; //variable for iteration
unsigned int l = 0; // var for storing size of buffer
unsigned int t=0; //temp var count corres to last valid byte 
int flag1=0; // for first byte validation
int flag=0; // for incrementing i
int flag3=0; //for eliminating chances of 251 entering as pressure
/************************************/
unsigned int  no_of_steps=0;
unsigned int DesiredPressure_mmHg[24]={};
unsigned int HoldingTime_msec=0;
unsigned int WaitingTime_msec=0;
unsigned int PressureIndex=0;
unsigned int PressureReleaseIndex=20;// added for pneumrelease delay control
unsigned int PressureStopIndex=19;//default value for default NIBP run

void setup() {
  Serial.begin(9600); // set serial speed
  pinMode(pwrPump0, OUTPUT);       // Pneumatic Pump Control
  pinMode(pwrPump1, OUTPUT);       // Pneumatic Pump Control
  pinMode(pwrValve0, OUTPUT);     // Pneumatic Release Valve
  pinMode(pwrValve1, OUTPUT);     // Pneumatic Valve2
  
  pinMode(switchA, INPUT);        // Run/Pause Button
  pinMode(switchB, INPUT);        // Halt Button

  digitalWrite(pwrPump0, LOW);
  digitalWrite(pwrPump1, LOW);
  digitalWrite(pwrValve0, LOW);
  digitalWrite(pwrValve1, LOW);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // added for OLED
}

void loop() {

/**************************  serial communication  *************************************************/  
flag=0; 
flag3=0; //? for serial communication first pressure value error check
 
if(flag1==0 and Serial.available()>0){   /*waiting for first valid byte */
  Serial.readBytes(buff,1);
  if(buff[0]==251){
    flag1=1; //start byte received
  }
}

l=Serial.available(); /*l=length of buffer after first byte has been read from buffer*/
// Store the pressure values, wt time, hld time and stop byte in buffer after receiving the start byte
if(Serial.available()>0 and i<Serial.available()and (flag1==1))
{
buff[0]=Serial.read();//?
if(buff[0]!=251){//? error check for first pressure value 
buffers[i]=buff[0]; 
flag3=1;
}
if(buffers[i]==255){ //read till 255-stop byte after which it is ready for populating pressure values
int t=i;
l=t+1;  //use temp variable to ensure l is exactly the length of all bytes received except first byte. that is (251,p1 ,.... p4,wt time, hld time, 255 --> i=6, l=7)
i=0; // reinitialise i for next profile chosen by the user.
flag=1;// used for indicating all valid bytes have been received
flag1=0;//reset for next profile to wait for start byte
for(int y=t+1; y<=(26);y++){
buffers[y]=0; // fill remaning buffer with zeros.
}   
}
if(flag==0 and flag3==1){//?
i=i+1;
}
}

/** populating pressures after fully receiving all valid data**/
if(flag==1)
{
PressureStopIndex = l-4;
PressureReleaseIndex=l-3;
(no_of_steps) = (l-3);
for(int  k=0;k<=(no_of_steps-1);k++){
DesiredPressure_mmHg[k]=buffers[k];
}
HoldingTime_msec=(buffers[l-2])*1000;
WaitingTime_msec=(buffers[l-3])*1000;
}

/** populating pressures after fully receiving all valid data**/

else if (l==0){  //default condition for pressures for manual press
PressureReleaseIndex=16;
PressureStopIndex = 15;
no_of_steps =16;
DesiredPressure_mmHg[0]= 30;
DesiredPressure_mmHg[1]= 40;
DesiredPressure_mmHg[2]= 50;
DesiredPressure_mmHg[3]= 60;
DesiredPressure_mmHg[4]= 70;
DesiredPressure_mmHg[5]= 80;
DesiredPressure_mmHg[6]= 90;
DesiredPressure_mmHg[7]= 100;
DesiredPressure_mmHg[8]= 110;
DesiredPressure_mmHg[9]= 120;
DesiredPressure_mmHg[10]= 130;
DesiredPressure_mmHg[11]= 140;
DesiredPressure_mmHg[12]= 150;
DesiredPressure_mmHg[13]= 160;
DesiredPressure_mmHg[14]= 170;
DesiredPressure_mmHg[15]= 180;
//DesiredPressure_mmHg[16]= 190;
//DesiredPressure_mmHg[17]= 200;
//DesiredPressure_mmHg[18]= 210;
//DesiredPressure_mmHg[19]= 220;
HoldingTime_msec=400;
WaitingTime_msec=0;
flag1=1;
}


/************************************************* NIBP program control **********************************************/

    curTime=millis();
    if(((digitalRead(switchA)!=swAstate) && ((curTime-swAtime)>100))|| (flag==1)&& DesiredPressure_mmHg[l-4]<200 ) //after populating all pressures which happens once start has been clicked from UI or start has been pressed from board manually
    {     // check switch press or release, with a debounce interval of 50ms, //added on aug 8
      swAstate=!swAstate;
      swAtime=curTime;
      if(swAstate==0) {       // switch has been pressed
        digitalWrite(pwrValve0, HIGH);
        digitalWrite(pwrValve1, HIGH);
        digitalWrite(pwrPump0, LOW);
        digitalWrite(pwrPump1, LOW);
        
        if(MachineState==1)          // if MachineState is 1, then toggle Pause/Run
        {
          ExptPhase=0;
          PhaseStartTime[ExptPhase]=curTime;
             if(PressureIndex>PressureStopIndex) 
          {
            PressureIndex=0;
            MachineState=0;
          }
        }             // done MachineState=1
        else if(MachineState==0) 
        {        // if MachineState is 0, then fresh start of experiment
          MachineState=1;
          PressureIndex=0;
          DisplayOLED(PressureIndex);   
          ExptPhase=0;
          PhaseStartTime[ExptPhase]=curTime;
        }             // done MachineState=0
      }               // done servicing switch press
    }                 // done checking switchA
    
    if((digitalRead(switchB)!=swBstate) && ((curTime-swBtime)>100) ) 
    {     // check switch press or release, with a debounce interval of 50ms
      swBstate=!swBstate;
      swBtime=curTime;
      if(swBstate==0) 
      {       // switch has been pressed
        digitalWrite(pwrValve0, HIGH);
        digitalWrite(pwrValve1, HIGH);
        digitalWrite(pwrPump0, LOW);
        digitalWrite(pwrPump1, LOW);
        MachineState=0;
        PressureIndex=0;
        ExptPhase=0;
        PhaseStartTime[ExptPhase]=curTime;
      }               // done servicing switch press
   
    
    
    }                 // done checking switchB
    
    if(MachineState==0) 
    {
      digitalWrite(pwrValve0, HIGH);
      digitalWrite(pwrValve1, HIGH);
      DisplayOLED(100);
      PressureIndex=0;
      ExptPhase=0;
      PhaseStartTime[ExptPhase]=curTime;   // initialize the timer for the next phase
    }
    
    else if(MachineState==1) 
    {
      switch(ExptPhase) 
      {  
         case PneumPumping: 
         {      
            if(PressureIndex!=0){ // specially written for first pressure display compensation
                    DisplayOLED(PressureIndex);
                  }
            MachineState==0;
//          CuffPressure_mmHg = ((analogRead(Pressure_ADCport))*0.3627+5);    // original calib eqn
            CuffPressure_mmHg = ((analogRead(Pressure_ADCport))*0.4098); //16.3    //0.4098-16.3 added, //0.3665, -6.3
            PumpONtime=curTime-PhaseStartTime[ExptPhase];
              digitalWrite(pwrPump0, HIGH);
              digitalWrite(pwrPump1, HIGH);
//               if(PressureIndex!=0){
//              if((CuffPressure_mmHg-9 > (DesiredPressure_mmHg[PressureIndex]))||(PumpONtime>maxpumpTime))  /// added for display -9 compensation
//              {
//                  digitalWrite(pwrPump0, LOW);
//                  digitalWrite(pwrPump1, LOW);
//              }}
             
              if((CuffPressure_mmHg> (DesiredPressure_mmHg[PressureIndex]))||(PumpONtime>maxpumpTime))  /// added for display -9 compensation
              {
                  digitalWrite(pwrPump0, LOW);
                  digitalWrite(pwrPump1, LOW);
              }
                  ExptPhase=PneumHolding;      // next phase
                  if(PressureIndex==0){
                    CuffPressure_mmHg = ((analogRead(Pressure_ADCport))*0.4098-7+(DesiredPressure_mmHg[0]-CuffPressure_mmHg)); 
                    DisplayOLED(PressureIndex); // condition for zero display as there is  display error due to adc variation from previous pressures.
                  }
                  while((((analogRead(Pressure_ADCport))*0.4098) <(DesiredPressure_mmHg[PressureIndex]))); // WAIT TILL MAX PRS IS BUILT UP, removed -7 -9 and +9, added on aug 8 2018
                  PhaseStartTime[ExptPhase]=curTime;   // initialize the timer for the next phase
                        
              }             // done ExptPhase=1
        break;

        case PneumHolding: 
        {   
//        CuffPressure_mmHg = ((analogRead(Pressure_ADCport))*0.3627+5);    
          CuffPressure_mmHg = ((analogRead(Pressure_ADCport))*0.4098);   
          if((CuffPressure_mmHg-11) < DesiredPressure_mmHg[PressureIndex]+25)// added for display -9 compensation
          {
            digitalWrite(pwrPump0, HIGH);
            digitalWrite(pwrPump1, HIGH);
            delayMicroseconds(400); // added for any pressure leaks
            digitalWrite(pwrPump0, LOW);
            digitalWrite(pwrPump1, LOW);
          }
          if((curTime-ledBflicktime)>20) 
          {       // blink LED to show that ExperimentState=2
            ledBflicktime=curTime;
            ledBToggle=!ledBToggle;
          }
           
          if((millis()-PhaseStartTime[ExptPhase])>HoldingTime_msec+500) 
          {
            if(PressureIndex>=0 && PressureIndex<PressureReleaseIndex){
              //delay(1000);
              ExptPhase=PneumPumping;               // holding time done; next phase
              PhaseStartTime[ExptPhase]=curTime;    // initialize the timer for the next phase
              PressureIndex++;
              if(PressureIndex>PressureStopIndex) 
              {
                PressureIndex=0;
                MachineState=0;
                flag3=0;//?
                flag1=0;//?
                flag-0;//?
                l=0;//?
                
              }}           
                 
          }
         }           // done ExptPhase=2
        break;
      
        case PneumRelease: 
          { 
          DisplayOLED(30);   
          if(PressureIndex>=0 && PressureIndex<PressureReleaseIndex)
          {
            digitalWrite(pwrValve0, HIGH);
            digitalWrite(pwrValve1, HIGH);
            while(((analogRead(Pressure_ADCport))*0.3665)>17); //WAIT FOR ,MIN PRESSURE
            PhaseStartTime[ExptPhase]=curTime;
            while((millis()-PhaseStartTime[ExptPhase])< WaitingTime_msec);
//            if((curTime-PhaseStartTime[ExptPhase])> WaitingTime_msec)
//            { 
             // delay(1000);           
              ExptPhase=PneumPumping;
              PhaseStartTime[ExptPhase]=curTime;   // initialize the timer for the next phase
              digitalWrite(pwrValve0, LOW);
              digitalWrite(pwrValve1, LOW);
//            }
          }
          
          }           
        break;
      }            
   }         
}

int DisplayOLED(int number) 
{
   switch (number) 
  {
    case 0: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[0]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);

display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 1: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[1]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);

display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 2: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[2]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);

display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 3: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[3]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 4: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[4]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 5: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[5]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 6: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[6]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 7: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[7]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 8: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[8]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 9: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[9]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 10: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[10]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;



case 11: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[11]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 12: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[12]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;



case 13: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[13]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;



case 14: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[14]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;



case 15: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[15]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;



case 16: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[16]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 17: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[17]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 18: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[18]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 19: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[19]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 20: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[20]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 21: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[21]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 22: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[22]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 23: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[23]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;


case 24: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(30,3);
display.write(83);
display.write(69);
display.write(84);
display.write(58);
p= int(DesiredPressure_mmHg[24]);
asciip(p);
display.write(disp(p3));
display.write(disp(p2));
display.write(disp(p1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);


display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(15,15);
CuffPressure_mmHg=CuffPressure_mmHg-20;
x= int(CuffPressure_mmHg);
ascii(x);
display.write(disp(x3));
display.write(disp(x2));
display.write(disp(x1));
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;

case 30: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(14,15);
//display.write(80);
//display.write(32);
//display.write(61);
//display.write(32);
display.write(48);
display.write(32);
display.write(109);
display.write(109);
display.write(72);
display.write(103);
display.display();
break;
      
default: 
display.clearDisplay();
display.setTextColor(WHITE);
display.setTextSize(2);
display.setCursor(48,3);
display.write(78);
display.write(73);
display.write(66);
display.write(80);
display.setTextColor(WHITE);
display.setTextSize(1);
display.setCursor(33,22);
display.write(67);
display.write(77);
display.write(67);
display.write(32);
display.write(86);
display.write(101);
display.write(108);
display.write(108);
display.write(111);
display.write(114);
display.write(101);
display.display();
         break;
  }
}

int ascii(int prs) //FOR ASCII OF SENSOR PRESSURE :CUFF PRS
{
     x1= (x%10) + 48;
     x11= x/10;
     x2= (x11%10) + 48;
     x22= x11/10;
     x3= (x22%10) + 48;
     x33= x22/10;

}

int asciip(int prs) // FOR ASCII OF SET PRESSURE :DESIRED PRS
{
     p1= (p%10) + 48;
     p11= p/10;
     p2= (p11%10) + 48;
     p22= p11/10;
     p3= (p22%10) + 48;
     p33= p22/10;

}


int disp(int pressure)
{
    return (pressure);
}
