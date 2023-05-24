// DDS with an analog control on a Eurocard standard rack developped by Juntian Tu at JQI in Jan. 2023
// The required SetListArduino library is inherited from https://github.com/JQIamo/SetListArduino-arduino.git

#include <SetListArduino.h>
#include <SPI.h>
#include <EEPROM.h>

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
  EEPROM.begin(256);
  SPI1.setCS(DDS0_CS);
  SPI1.begin(); // SPI is begun before lcd intentionally since I used the MISO pin for the lcd control
  delay(50);
  Serial1.setRX(SERIAL_RX);
  Serial1.setTX(SERIAL_TX);
  Serial1.begin(115200);  // Used for communicating with other microcontroller
  delay(50);
  pinMode(LCD_RST, OUTPUT);
  digitalWrite(LCD_RST, HIGH);
  SPI.setTX(LCD_MOSI);
  SPI.setSCK(LCD_CLK);
  delay(100);  
  encoder.setup();
  lcd.begin();
  lcd.cursor();
  
  


  char name_holder[17];
  channel_index = EEPROM.read(0);  // We use the channel index instead of the profile number to distinguish the DDSs
  sprintf(name_holder,"Channel:%1u",channel_index);
  // Setting up the menus
  channel_set.current_channel = channel_index;
  strcpy(channel_set._display_name,name_holder);

  root.add(&analog_switch);
  root.add(&analog_setting);
  root.add(&static_out);
  root.add(&channel_set);
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
int is_master = 0; // 0 for non-determined; 1 for master; 2 for slave
char serial_buffer[SERIAL_BUFFER_SIZE];


void loop() {
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

   }else{
    if (is_master != 2){
      SetListImage.readSerial(0); // 0 is the USB Serial; 1 is the Serial pin
      if (SetListImage.get_buffer()[0]=='\0'){
        if(is_master == 0){
          SetListImage.readSerial(1);
          if (SetListImage.get_buffer()[0]!='\0'){
            Serial1.addMemoryForRead(serial_buffer,SERIAL_BUFFER_SIZE);
            is_master = 2;
          }
        }
      }else{ 
        delay(50); // Need some time for transferring from receiving to tranferring
        Serial1.print(SetListImage.get_buffer());
        if (is_master == 0){
          is_master = 1;
          Serial1.addMemoryForWrite(serial_buffer,SERIAL_BUFFER_SIZE);
        }
      }
    }else{
      SetListImage.readSerial(1);
    }
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
