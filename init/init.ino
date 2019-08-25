#define ENCODER0PINA         2    //Atmega pin PD2/INT0 // interrupt pin (2,3 on Atmega328)
#define ENCODER1PINA         3    //Atmega pin PD3/INT1, pin 3 also PWM paired with 11.
#define pinENA               5    //Atmega pin PD5 //PWM pin, grouped with pin 6
#define pinINA1              7    //Atmega pin PD7
#define pinINA2              8    //Atmega pin PB0
#define pinENB               9    //Atmega pin PB1 //PWM pin, grouped with pin 10
#define pinINB1              12   //Atmega pin PB4
#define pinINB2              13   //Atmega pin PB5


long encoder0Position = 0;
long encoder1Position = 0;
int PPR=8; //pulses per rotation
int PWMA = 140;
int PWMB = 140;
// volatile variables - modified by interrupt service routine (ISR)
volatile long counter0=0;
volatile long counter1=0;
bool motorARunning = false;
bool motorBRunning = false;
long stepsA;
long stepsB;
char dirA;
char dirB;
//Serial Read stuff
const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data
boolean newData = false;
char DIR[1] = {0};
//char DIR = 'Z';
long VALUE = 0;


void setup()
{
 
  // pins
  pinMode(ENCODER0PINA, INPUT_PULLUP);
  pinMode(ENCODER1PINA, INPUT_PULLUP);
  pinMode(pinENA,OUTPUT);
  pinMode(pinINA1,OUTPUT);
  pinMode(pinINA2,OUTPUT);
  pinMode(pinENB,OUTPUT);
  pinMode(pinINB1,OUTPUT);
  pinMode(pinINB2,OUTPUT);
  //attach interrupts 
  attachInterrupt(digitalPinToInterrupt(ENCODER0PINA),onInterrupt0, RISING);
  attachInterrupt(digitalPinToInterrupt(ENCODER1PINA),onInterrupt1, RISING); 
  Serial.begin (57600);

  delay(1000);
  analogWrite(pinENA, PWMA);
  digitalWrite(pinINA1,LOW);
  digitalWrite(pinINA2,LOW);
  
  analogWrite(pinENB, PWMB);
  digitalWrite(pinINB1,LOW);
  digitalWrite(pinINB2,LOW);

}
 
void loop()
{
  //Read the Serial and move accordingly
  recvWithStartEndMarkers();
  
  //showNewData();

      setPosition(DIR[0],VALUE);
      
  
  
  
  
}
// Minimalistic ISR

void onInterrupt0()
{

  counter0++;

}
void onInterrupt1()
{
  counter1++;

}
void rotateAF()
{
    analogWrite(pinENA, PWMA);
    digitalWrite(pinINA1,HIGH);
    digitalWrite(pinINA2,LOW);
    motorARunning=true;
}
void rotateAR()
{
    analogWrite(pinENA, PWMA);
    digitalWrite(pinINA1,LOW);
    digitalWrite(pinINA2,HIGH);
    motorARunning=true;
}
void rotateAStop()
{
  analogWrite(pinENA, 0);
  digitalWrite(pinINA1,LOW);
  digitalWrite(pinINA2,LOW);
  motorARunning=false;
}

void rotateBF()
{
    analogWrite(pinENB, PWMB);
    digitalWrite(pinINB1,HIGH);
    digitalWrite(pinINB2,LOW);
    motorBRunning=true;
}
void rotateBR()
{
    analogWrite(pinENB, PWMB);
    digitalWrite(pinINB1,LOW);
    digitalWrite(pinINB2,HIGH);
    motorBRunning=true;
}
void rotateBStop()
{
  analogWrite(pinENB, 0);
  digitalWrite(pinINB1,LOW);
  digitalWrite(pinINB2,LOW);
  motorBRunning=false; 

}

//Serial Read Functions
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
 // if (Serial.available() > 0) {
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                parseData();
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

void showNewData() {
    if (newData == true) {
        Serial.print("This just in ... ");
        Serial.println(receivedChars);
        Serial.print("DIRECTION=");
        Serial.println(DIR);
        Serial.print("VALUE=");
        Serial.println(VALUE);
        newData = false;
    }
}


 
void parseData() {

    // split the data into its parts
    
  char * strtokIndx; // this is used by strtok() as an index
  
  strtokIndx = strtok(receivedChars,"-");      // get the first part - the string
  strcpy(DIR, strtokIndx); // copy it to DIR
  //strcpy(DIR,0);
  //strtokIndx = strtok(NULL, "-"); // this continues where the previous call left off
  //VALUE = atoi(strtokIndx); // convert this part to an integer
  VALUE = atol(strtokIndx+2);   
 }

void setPosition(char dir, long steps)
{


  if (newData)
  {
    // It is new data from Serial - it wants to move the motors one way or the other
    // if stepsA<offshootA - no point turning the motor. Repeat for stepsB     
         if (dir=='F') //Single quotes - Double quotes don't work.
            {
                if (not (motorARunning)) {rotateAF(); stepsA=steps; dirA=dir;}
             }
         else if (dir=='R')
            {
               if (not (motorARunning)) {rotateAR();stepsA=steps;dirA=dir;}
            }
         else if (dir=='f') //Single quotes - Double quotes don't work.
            {
               rotateBF();
               // if (not (motorBRunning)) {rotateBF();stepsB=steps;dirB=dir;}
             }
         else if (dir=='r')
            {
               rotateBR();
               //if (not (motorBRunning)) {rotateBR();stepsB=steps;dirB=dir;}
            }
    newData=false;
    }

    //Now motors are running - count steps
    if ((motorARunning) and (stepsA<counter0))
            {
             rotateAStop();
             
             
             if (dirA=='F')
            {
                encoder0Position = encoder0Position+counter0;

             }
             else if (dirA=='R')
            {
               encoder0Position = encoder0Position-counter0;
               
            }
            
        counter0 =0;
        Serial.print("0-");
        Serial.println(encoder0Position); 
            }
     
    if ((motorBRunning) and (stepsB<counter1))
    {
              rotateBStop();
              
              
             if (dirB=='f')
            {
                
                encoder1Position = encoder1Position+counter1;
             }
         else if (dirB=='r')
            {
               
               encoder1Position = encoder1Position-counter1;
            }
        counter1 =0;
        Serial.print("1-");
        Serial.println(encoder1Position); 
            }
     


}
