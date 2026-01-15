// DDS with an analog control on a Eurocard standard rack developped by Juntian Tu at JQI in Jan. 2023
// The required SetListArduino library is inherited from https://github.com/JQIamo/SetListArduino-arduino.git

#include <SetListArduino.h>
#include <SPI.h>
#include <EEPROM.h>

#include "LCD.h"
#include "settings.h"
#include "pin_assign.h"
#include <AD9910.h>
#include "encoder.h"


const int if_serial = 0;
int analog_amp;
int analog_freq;
int is_master = 0; // 0 for non-determined; 1 for master; 2 for slave
char serial_buffer[SERIAL_BUFFER_SIZE];
SetListArduino SetListImage(SETLIST_TRIG);
int * loop_param;
int is_loop=0;


//Prototype functions
void setFreq0(AD9910 * dds, int * params);
void setAmp0(AD9910 * dds, int * params);
void FreqLoop0(AD9910 * dds, int * params);
void setWave0(AD9910 * dds, int * params);
void setAnalogMode0(AD9910 * dds, int * params);
void followAnalog0(AD9910 dds, int analog_freq,int analog_amp);
void shrinkFM0(AD9910 * dds, int * params);
void setFM0(AD9910 * dds, int * params);
void setup() {
  is_loop = 0;
  delay(1000);
  SPI.begin(); // SPI is begun before lcd intentionally since I used its MISO pin for the lcd control
  delay(50);
  if (if_serial) {
    Serial5.begin(115200);  // Used for communicating with other microcontrollers
    delay(50);
  }
  pinMode(LCD_RST, OUTPUT);
  digitalWriteFast(LCD_RST, HIGH);
  delay(100);  
  encoder.setup();
  lcd.begin();
  lcd.cursor();

  char name_holder[17];
  channel_index = EEPROM.read(0);  // We use the channel index instead of the profile number to distinguish the DDSs
  sprintf(name_holder,"Channel:%1u",channel_index);
  // Setting up the menus
  channel_set.current_channel = channel_index; // Assign the display name of channel_set menu
  strcpy(channel_set._display_name,name_holder);

  root.add(&analog_switch);
  root.add(&analog_setting);
  root.add(&static_out);
  root.add(&static_out_amp);
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

  DDS0.initialize(40000000,25);
  delay(100);

  DDS0.setWave(DEFAULT_FREQ,0,DEFAULT_AMP);
  delay(10);
  
  // SetListImage.registerDevice(DDS0, channel_index);
  SetListImage.registerDevice(DDS0, 0);
  
  SetListImage.registerCommand("f", 0, setFreq0);
  SetListImage.registerCommand("w", 0, setWave0);
  SetListImage.registerCommand("a", 0, setAnalogMode0);
  SetListImage.registerCommand("am", 0, setAmp0);
  SetListImage.registerCommand("fl", 0, FreqLoop0);
  SetListImage.registerCommand("fm", 0, setFM0);
  SetListImage.registerCommand("sf", 0, shrinkFM0);
  if (if_serial){
    Serial5.setTX(20);
    Serial5.setRX(21);
    Serial5.begin(115200);
  }
  if (!if_serial) {is_master = 2;}
  root.enter();
}





void loop() {
  if (is_loop){
    FreqLoop0(&DDS0,loop_param);
  }else{
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
    if (is_master != 2 or !if_serial){
      SetListImage.readSerial(); // 0 is the USB Serial; 5 is the Serial pin
      if (SetListImage.get_buffer()[0]=='\0'){
        if(is_master == 0){
          Serial5.addMemoryForRead(serial_buffer,SERIAL_BUFFER_SIZE);
          SetListImage.readSerialH(5);
          if (SetListImage.get_buffer()[0]!='\0'){
            is_master = 2;
          }
        }
      }else{ 
        delay(50); // Need some time for transferring from receiving to tranferring
        if (is_master == 0){
          is_master = 1;
          Serial5.addMemoryForWrite(serial_buffer,SERIAL_BUFFER_SIZE);
        }
        if (if_serial){
        Serial5.print(SetListImage.get_buffer());
        Serial5.flush();
        }
      }
    }else if (if_serial){
      SetListImage.readSerialH(5);
    }
  }
}}

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
        // char kk[20] = "";
        // sprintf(kk, "k%u",amp);
        // lcd.setCursor(0, 0);
        // lcd.printer(kk);
  if (amp> 100){amp=100;}
  dds.setWave(freq,0,amp);
  // Serial.println(micros()-a); Takes around 5 us per loop
}

void setFreq0(AD9910 * dds, int * params){
  dds->isAnalogMode = false;
  double dfreq = (double)params[0]; //(frequency, in Hz)
  int freq = (int)(dfreq);
  dds->setFreq(freq);
}

void setAmp0(AD9910 * dds, int * params){
  dds->isAnalogMode = false;
  double damp = (double)params[0]; 
  int amp = (int)(damp);
  dds->setAmp(amp);
}

void FreqLoop0(AD9910 * dds, int * params){
  if (!is_loop){
  dds->isAnalogMode = false;
  is_loop = 1;
  loop_param = params;
  }
  double dfreq1 = (double)params[0]; //(frequency, in Hz)
  int freq1 = (int)(dfreq1);
  dds->setFreq(freq1);
  double dfreq2 = (double)params[1]; //(frequency, in Hz)
  int freq2 = (int)(dfreq2);
  dds->setFreq(freq2);
}


void setWave0(AD9910 * dds, int * params){
  dds->isAnalogMode = false;
  double dfreq = (double)params[0]; //(frequency, in Hz)
  int freq = (int)(dfreq);
  int phase_offset = params[1];
  int amp = params[2];
  dds->setWave(freq,phase_offset,amp);
}


void setFM0(AD9910 * dds, int * params){
  dds->isDRG = 1;
  dds->isAnalogMode = false;
  double center_freq = (double)params[0]; //(frequency, in Hz)
  double mod_deviation = (double)params[1]; //(frequency, in Hz)
  double mod_freq = (double)params[2]; //(frequency, in Hz)
  double step_size = (double)params[3]; //(frequency, in Hz)
  dds->setFM(center_freq, mod_deviation, mod_freq, step_size);
}

void shrinkFM0(AD9910 * dds, int * params){
  dds->isAnalogMode = false;
  dds->isDRG = 1;
  double new_deviation = (double)params[0]; //(frequency, in Hz)
  dds->shrinkFM(new_deviation);
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
