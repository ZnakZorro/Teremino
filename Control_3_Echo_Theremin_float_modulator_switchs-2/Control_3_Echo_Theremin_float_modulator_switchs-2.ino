
//version6
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <RollingAverage.h>
#include <ControlDelay.h>
#include <mozzi_rand.h>
#include <ADSR.h>
#include <mozzi_midi.h>

#define INPUT_PIN0 0
#define INPUT_PIN1 1
#define INPUT_PIN2 2
#define dpinIN0 11
#define dpinIN1 12

#define CONTROL_RATE 64

unsigned int echo_cells_1 = 32;
unsigned int echo_cells_2 = 64;
unsigned int echo_cells_3 = 128;
unsigned int echo_cells_4 = 256;

ControlDelay <128, int> kDelay; // 2seconds

// oscils to compare bumpy to averaged control input
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin0(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin1(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin2(SIN2048_DATA);
Oscil <SIN2048_NUM_CELLS, AUDIO_RATE> aSin3(SIN2048_DATA);

Oscil<SIN2048_NUM_CELLS, CONTROL_RATE> modulator1(SIN2048_DATA);
Oscil<SIN2048_NUM_CELLS, CONTROL_RATE> modulator2(SIN2048_DATA);

// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 32> kAverage; // how_many_to_average has to be power of 2
int averaged;
byte pinpin=1;

float mfrq1=3.0;
float mfrq2=0.1;


void setup(){
  pinMode(dpinIN0, INPUT_PULLUP);
  pinMode(dpinIN1, INPUT_PULLUP);
  //Serial.begin(9600);
  kDelay.set(echo_cells_1);
  startMozzi();
  
  modulator1.setFreq(mfrq1);
  modulator2.setFreq(mfrq2); 
}

int licznik=0;

void updateControl(){
  licznik++;
       byte pinpin1 = (digitalRead(dpinIN0) && 1) <<1;
       byte pinpin2 = (digitalRead(dpinIN1) && 1);
       pinpin = pinpin1+pinpin2;
   //if (licznik % 128 == 0) {pinpin = rand(4);pinpin++;}  // symulacja
      
   //Serial.print(digitalRead(dpinIN0));   
   //Serial.print(digitalRead(dpinIN1));   
   //Serial.print(" pinpin="); Serial.print(pinpin); 
   //mfrq1 = 2.0 / (pinpin);
   //mfrq2 = 0.2 * (pinpin);
  
  long ADCval_0 = (3*mozziAnalogRead(INPUT_PIN0));
  long ADCval_1 = mozziAnalogRead(INPUT_PIN1);
  long ADCval_2 = mozziAnalogRead(INPUT_PIN2);

  float mult1 = (float) ADCval_1/743;
  float mult2 = (float) ADCval_2/743;

  modulator1.setFreq(((mult1)/1)* mfrq1);
  modulator2.setFreq(((mult2)/1)* mfrq2);
  
  averaged = (kAverage.next((ADCval_0+(modulator1.next()<<2)))* (((modulator2.next()+128)>>6)));
  /*
  Serial.print(mult1);Serial.print("\t");
  Serial.print(mult2);Serial.print("\t");
  Serial.print(fval);Serial.print("\t");
  Serial.print(val);Serial.print("\t");
  */
  
  //if (ADCval_0 < 26) averaged=0;
  //Serial.print("\t"); Serial.print(ADCval_0);
  //Serial.println("");


/****ADCval_0***************************************************/
      switch (pinpin) {
        case 0: {aSin0.setFreq(averaged);}break;
        //case 1: {aSin0.setFreq((licznik << 8) * averaged);}break;
        case 1: {byte midi_note = 87-(licznik % 50);  aSin0.setFreq((int)mtof(midi_note));}break;
        case 2: {byte midi_note = 127-(licznik % 100);  aSin0.setFreq((int)mtof(midi_note));}break;
        case 3: {byte midi_note = (licznik % 107)+20;  aSin0.setFreq((int)mtof(midi_note));}break;
        //case 2: {byte midi_note = 80-((licznik+ADCval_0>>4) % 20);  aSin0.setFreq((int)mtof(midi_note));}break;
        //case 3: {byte midi_note = ((licznik+ADCval_0>>4) % 20)+60;  aSin0.setFreq((int)mtof(midi_note));}break;
        //case 2: {aSin0.setFreq((licznik % 73) + (int)ADCval_0);}break;
        //case 3: {aSin0.setFreq((int)ADCval_0 - (licznik % 73));}break;
        
        
       }
/*********************************************************/



  
  //aSin0.setFreq(averaged);
  aSin1.setFreq(kDelay.next(averaged));
  aSin2.setFreq(kDelay.read(echo_cells_1));
  aSin3.setFreq(kDelay.read(echo_cells_3));
}


int updateAudio(){
  //3*
  return 3*((int)aSin0.next()+aSin1.next()+(aSin2.next()>>1)
    +(aSin3.next()>>2)) >>3;
}


void loop(){
  audioHook();
}
