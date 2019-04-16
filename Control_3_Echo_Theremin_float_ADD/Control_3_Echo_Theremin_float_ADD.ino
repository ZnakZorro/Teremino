
//version2
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <RollingAverage.h>
#include <ControlDelay.h>
#include <mozzi_rand.h>

#define INPUT_PIN0 0
#define INPUT_PIN1 1
#define INPUT_PIN2 2
#define CONTROL_RATE 64

unsigned int echo_cells_1 = 32;
unsigned int echo_cells_2 = 60;
unsigned int echo_cells_3 = 127;

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

void setup(){
   Serial.begin(9600);
  kDelay.set(echo_cells_1);
  startMozzi();
  
  modulator1.setFreq(0.333f);  //=0.333f
  modulator2.setFreq(0.25f);  //=0.333f
}


void updateControl(){
  long ADCval_0 = (2*mozziAnalogRead(INPUT_PIN0));
  long ADCval_1 = mozziAnalogRead(INPUT_PIN1);
  long ADCval_2 = mozziAnalogRead(INPUT_PIN2);
  //float mult1 = (rand(ADCval_1)/100)+2;
  //float mult2 = (rand(ADCval_2)/100)+2;

  float mult1 = ((((float)modulator1.next()+127)/256) * ((float) ADCval_1)/743);
  float mult2 = ((((float)modulator2.next()+127)/256) * ((float) ADCval_2)/743);
  float fval = (float) ADCval_0 + (ADCval_1+mult1) + (ADCval_2+mult2);
  long val =  fval;

  //modulator1.setFreq(((mult1)/1)* 1.1f);
  //modulator2.setFreq(((mult2)/1)* 0.1f);
  
  averaged = kAverage.next(val);
  /*
  Serial.print(mult1);Serial.print("\t");
  Serial.print(mult2);Serial.print("\t");
  Serial.print(fval);Serial.print("\t");
  Serial.print(val);Serial.print("\t");
  Serial.println(averaged);
  */
  aSin0.setFreq((int)ADCval_0+averaged);
  aSin1.setFreq(kDelay.next(averaged));
  aSin2.setFreq(kDelay.read(echo_cells_2));
  aSin3.setFreq(kDelay.read(echo_cells_3));
}


int updateAudio(){
  return 3*((int)aSin0.next()+aSin1.next()+(aSin2.next()>>1)
    +(aSin3.next()>>2)) >>3;
}


void loop(){
  audioHook();
}
