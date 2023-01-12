// DDS with an analog control on a Eurocard standard rack developped by Juntian Tu at JQI in Jan. 2023
// The required SetListArduino library is inherited from https://github.com/JQIamo/SetListArduino-arduino.git

#include <SetListArduino.h>
#include <SPI.h>

#include "LCD.h"
#include "menus.h"
#include "pin_assign.h"
#include <AD9910.h>
#include "encoder.h"

SetListArduino SetListImage(SETLIST_TRIG);

//Prototype functions
void setFreq0(AD9910 * dds, int * params);
void setWave0(AD9910 * dds, int * params);
void setAnalogMode0(AD9910 * dds, int * params);
void followAnalog0(AD9910 dds, int analog_freq,int analog_amp);
void setup() {
  delay(1000);
  SPI.begin(); // SPI is begun before lcd intentionally since I used the MISO pin for the lcd control
  delay(50);
  pinMode(LCD_RST, OUTPUT);
  digitalWriteFast(LCD_RST, HIGH);
  SPI1.begin();
  delay(100);  
  encoder.setup();
  lcd.begin();
  lcd.cursor();

  root.add(&analog_switch);
  root.add(&analog_setting);
  analog_setting.add(&freq_max);
  analog_setting.add(&freq_min);
  analog_setting.add(&back);

  delay(150);
// For AD9910 dds...
  pinMode(DDS0_RESET, OUTPUT);
  digitalWrite(DDS0_RESET, LOW);
  delay(100);
    
  digitalWrite(DDS0_RESET, HIGH);
  delay(1);
  digitalWrite(DDS0_RESET, LOW);

  DDS0.initialize(40000000,25); // TODO: Potentoally adding more DDS
  delay(100);

  DDS0.setFreq(10000000);
  delay(10);
 
  SetListImage.registerDevice(DDS0, 0);
  
  SetListImage.registerCommand("f", 0, setFreq0);
  SetListImage.registerCommand("w", 0, setWave0);
  SetListImage.registerCommand("a", 0, setAnalogMode0);

  root.enter();

}
int analog_amp;
int analog_freq;

void loop() {
   SetListImage.readSerial(); 
   char encoder_active = encoder.reader();
   if (encoder_active){
   root._active->process(encoder_active);
   }
   if (DDS0.isAnalogMode){
    // uint32_t a = micros();
    analog_amp = analogRead(ANALOG_AMP);
    analog_freq = analogRead(ANALOG_FREQ);
    // Serial.println(micros()-a); // The 2 Analog reads takes aroung 36 us per loop
    followAnalog0(DDS0,analog_freq,analog_amp);
   }
}

void setAnalogMode0(AD9910 * dds, int * params){
  dds->isAnalogMode = true;
}

void followAnalog0(AD9910 dds, int analog_freq,int analog_amp){
  // uint32_t a = micros();
  double minAnalogFreq, maxAnalogFreq;
  minAnalogFreq = (double) dds.minAnalogFreq;
  maxAnalogFreq = (double) dds.maxAnalogFreq;
  double dfreq = ((double)analog_freq * (maxAnalogFreq - minAnalogFreq)) * 4 / 3069. +minAnalogFreq;
  uint32_t freq = (uint32_t)(dfreq);
  uint32_t amp = (uint32_t)analog_amp * 400 / 3069 ;
  dds.setWave(freq,0,amp);
  // Serial.println(micros()-a); Takes around 5 us per loop
}

void setFreq0(AD9910 * dds, int * params){
  if(dds->isAnalogMode){
    dds->isAnalogMode = false;
  }
  double dfreq = (double)params[0]; //(frequency, in Hz)
  int freq = (int)(dfreq);
  dds->setFreq(freq);
}


void setWave0(AD9910 * dds, int * params){
  if(dds->isAnalogMode){
    dds->isAnalogMode = false;
  }
  double dfreq = (double)params[0]; //(frequency, in Hz)
  int freq = (int)(dfreq);
  int phase_offset = params[1];
  int amp = params[2];
  dds->setWave(freq,phase_offset,amp);
}








/* Unfinished/Untested/Unused functions


void setRamp(AD9954 * dds, int * params){
  int f0 = params[0];
  int f1 = params[1];
  int tau = params[2];
    
  int redf0 = f0/1000;
  int redf1 = f1/1000;
    
  int RR = 200;
  int posDF = ((redf1-redf0)*RR)/(tau*100);
  
  dds->linearSweep(f0,f1,posDF,RR,posDF,RR);
}


*/
