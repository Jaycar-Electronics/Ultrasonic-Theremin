//Ultrasonic Theremin using PWM to play samples
//Samples played at rate dependent on distance read by ultrasonic sensor
//Giving theremin-like effect

//Sample timer interrupt runs 16bit Timer 1 at FREQ Hz (try 22050)
//PWM timer runs at 62500Hz on 8bit Timer 2 (max possible with 16MHz processor)
//Timer 0 is left unaltered, and is needed for delay/millis in ultrasonic functions
//sample is held in the sample array (sine sample also included on sample.c)

#include "sample.c"
#define FREQ 22050

//Ultrasonic HC-SR04 unit interface
//define pins here
//if using fixed power pins, set to a negative number, and they will be ignored
#define UVCC 8
#define UTRIG 9
#define UECHO 10
#define UGND 11
volatile unsigned long d=0;
volatile unsigned long u=0;

void setup() {
  usonicsetup();
  TMRsetup();
}

void loop() {
  unsigned long t;
  t=usonic(5800);     //up to 1m
  t=5800-t;           //invert so closer to sensor is higher
  if(t>5700){t=0;}    //handle edge cases
  u=t*200L;            //50 gives closer to true sample rate, but doesn't sound as good
  delay(50);          //wait a little- try higher values here for a more granular/synthesised sound
}

ISR(TIMER1_COMPA_vect) {    //gets triggered FREQ times/second
    OCR2B=pgm_read_byte(&theremin[d>>18]);   //load sample
    d=d+u;                                 //step forward to next sample
}

void TMRsetup(){
  // Timer 1 set up as a FREQ Hz sample interrupt, only common thing this affects is servo library
  TCCR1A = 0;
  TCCR1B = _BV(WGM12) | _BV(CS10);
  TCNT1 = 0;
  OCR1A = F_CPU / FREQ;
  TIMSK1 = _BV(OCIE1A);
    
  //PWM output fixed to D3 on TMR2 at 62.5kHz, PWM duty cycle is set by OCR2B
  pinMode(3,OUTPUT);   
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS20);
  TIMSK2 = 0;

}
void usonicsetup(void){
  if(UGND>-1){                  //set up ground pin if defined
    pinMode(UGND, OUTPUT);
    digitalWrite(UGND, LOW);    
  }
  if(UVCC>-1){                  //set up VCC pin if defined
    pinMode(UVCC, OUTPUT);
    digitalWrite(UVCC, HIGH);    
  }
  pinMode(UECHO, INPUT);        //ECHO pin is input
  pinMode(UTRIG, OUTPUT);       //TRIG pin is output
  digitalWrite(UTRIG, LOW);     
}

long usonic(long utimeout){    //utimeout is maximum time to wait for return in us
  long b;
  if(digitalRead(UECHO)==HIGH){return 0;}    //if UECHO line is still low from last result, return 0;
  digitalWrite(UTRIG, HIGH);  //send trigger pulse
  delay(1);
  digitalWrite(UTRIG, LOW);
  long utimer=micros();
  while((digitalRead(UECHO)==LOW)&&((micros()-utimer)<1000)){}  //wait for pin state to change- return starts after 460us typically
  utimer=micros();
  while((digitalRead(UECHO)==HIGH)&&((micros()-utimer)<utimeout)){}  //wait for pin state to change
  b=micros()-utimer;
  return b;
}
