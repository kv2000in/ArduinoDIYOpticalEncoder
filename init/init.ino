#define ENCODER0PINA         2    //Atmega pin PD2/INT0 // interrupt pin (2,3 on nano)


long encoder0Position = 0;


// volatile variables - modified by interrupt service routine (ISR)
volatile long counter0=0;
volatile long counter1=0;



void setup()
{

// pins
pinMode(ENCODER0PINA, INPUT_PULLUP);

//attach interrupts 
attachInterrupt(digitalPinToInterrupt(ENCODER0PINA),onInterrupt0, RISING);
Serial.begin (9600);

delay(1000);

}

void loop()
{






}
// Minimalistic ISR

void onInterrupt0()
{
// read both inputs - Why?? Interrupt was triggered by RISING edge on a
// So we know that a =1 and that should be the case always.
// So just read b. If it is 0 - moving counterclockwise, if it is 1 - moving clockwise.
//Different readings of b at different speeds. When PWM set to 180 b = 1, position decreasing
//When PWM is set to 255 - b =0 and position increasing
//For the test setup motor - canon feed motor running from 24 V off canon psu - PWM = 200 seems to be transition point for same direction of rotation. 
//For PWM >200 when interrupt is triggerred - b = 0
//For PWM <200 when interrupt is triggerred - b = 1
// So, this would have to be individualized for different motors and encoders. 
//int a = digitalRead(ENCODER0PINA);
counter0++;
Serial.println(counter0);
}
