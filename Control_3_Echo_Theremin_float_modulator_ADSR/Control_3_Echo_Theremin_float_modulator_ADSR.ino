
//version5
#include <MozziGuts.h>
#include <Oscil.h> // oscillator template
#include <tables/sin2048_int8.h> // sine table for oscillator
#include <RollingAverage.h>
#include <ControlDelay.h>
#include <mozzi_rand.h>
#include <ADSR.h>
#include <mozzi_midi.h>
#include <EventDelay.h>

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

EventDelay noteDelay;
ADSR <AUDIO_RATE, AUDIO_RATE> envelope;
boolean note_is_on = true;

// use: RollingAverage <number_type, how_many_to_average> myThing
RollingAverage <int, 32> kAverage; // how_many_to_average has to be power of 2
int averaged;
byte pinpin=1;

float mfrq1=2.0;
float mfrq2=0.2;
byte midi_note;

void setup(){
  pinMode(dpinIN0, INPUT_PULLUP);
  pinMode(dpinIN1, INPUT_PULLUP);
  Serial.begin(9600);
  kDelay.set(echo_cells_1);
  startMozzi();
  midi_note = rand(107)+20;
  modulator1.setFreq(mfrq1);
  modulator2.setFreq(mfrq2); 
}

int licznik=0;
unsigned int duration, attack, decay, sustain, release_ms;

void updateControl(){
  licznik++;
       //if (digitalRead(STOP_PIN) == HIGH) 
       //pinpin = (digitalRead(dpinIN0) && 1) <<1;
       //pinpin = (digitalRead(dpinIN1) && pinpin);
       //pinpin++;
   if (licznik % 128 == 0) {pinpin = rand(4);pinpin++;}  // symulacja
      /*
      switch (pinpin) {
        //case 0: {Serial.print(0);Serial.print("\t");}break;
        case 1: {Serial.print("=1");Serial.print("\t");}break;
        case 2: {Serial.print("=2");Serial.print("\t");}break;
        case 3: {Serial.print("=3");Serial.print("\t");}break;
        case 4: {Serial.print("=4");Serial.print("\t");}break;
      }
      */
   Serial.print("pinpin="); Serial.print(pinpin); 
   mfrq1 = 2.0 / (pinpin*pinpin);
   mfrq2 = 0.2 * (pinpin*pinpin);
  
  long ADCval_0 = (5*mozziAnalogRead(INPUT_PIN0));
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
  if (licznik % 32==0){
    //Serial.println(averaged);
  }
      /*eeeeeeeeeeeeeeeeeeeeeeeeeeeeeee*/
if (licznik % 64==0){
           unsigned int new_value = rand(300) +100;
           Serial.println(new_value);
           // randomly choose one of the adsr parameters and set the new value
           switch (rand(4)){
             case 0:
             attack = new_value;
             break;
      
             case 1:
             decay = new_value;
             break;
      
             case 2:
             sustain = new_value;
             break;
      
             case 3:
             release_ms = new_value;
             break;
           }
      
      byte attack_level = rand(128)+127;
      byte decay_level = rand(255);
      envelope.setADLevels(attack_level,decay_level);
       
      envelope.setTimes(attack,decay,sustain,release_ms);
      envelope.noteOn();

      midi_note = rand(107)+20;}
     aSin0.setFreq((int)mtof(midi_note));
     /*eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee*/
  
  if (ADCval_0 < 26) averaged=0;
  Serial.print("\t"); Serial.print(ADCval_0);
  Serial.println("");
  //aSin0.setFreq(averaged);
  aSin1.setFreq(kDelay.next(averaged));
  aSin2.setFreq(kDelay.read(echo_cells_3));
  aSin3.setFreq(kDelay.read(echo_cells_4));
}


int updateAudio(){
  //3*
  envelope.update();
  return (int) (envelope.next() * aSin0.next())>>8;
  //return 3 *((int)aSin0.next()+aSin1.next()+(aSin2.next()>>1)    +(aSin3.next()>>2)) >>3;
}


void loop(){
  audioHook();
}
