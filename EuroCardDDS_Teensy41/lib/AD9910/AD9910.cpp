// AD9910 used for DDS with an analog control by Oliver/Juntian Tu at JQI in Jan. 2023
// Following is the comment in the original library this code is based on.

/*
   AD9910.cpp - AD9910 DDS communication library
   Based on AD9914 by Ben Reschovsky, 2016
   JQI - Strontium - UMD
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "AD9910.h"

/* CONSTRUCTOR */

// Constructor function; initializes communication pinouts
AD9910::AD9910(int ssPin, int masterResetPin, int updatePin, int ps0, int ps1, int ps2, int osk) // reset = master reset
{
    RESOLUTION  = 4294967295.0;
    _ssPin = ssPin;
    _resetPin = masterResetPin;
    _updatePin = updatePin;
    _ps0 = ps0;
    _ps1 = ps1;
    _ps2 = ps2;
    _osk = osk;
} 

    /* PUBLIC CLASS FUNCTIONS */


// initialize(refClk) - initializes DDS with reference freq, divider
void AD9910::initialize(unsigned long ref, uint8_t divider, bool reset){
    isAnalogMode = false;
    // sets up the pinmodes for output
    pinMode(_ssPin, OUTPUT);
    pinMode(_resetPin, OUTPUT);
    pinMode(_updatePin, OUTPUT);
    pinMode(_ps0, OUTPUT);
    pinMode(_ps1, OUTPUT);
    pinMode(_ps2, OUTPUT);
    pinMode(_osk, OUTPUT);

    // defaults for pin logic levels
    digitalWrite(_ssPin, HIGH);
    digitalWrite(_resetPin, LOW);
    digitalWrite(_updatePin, LOW);
    digitalWrite(_ps0, LOW);
    digitalWrite(_ps1, LOW);
    digitalWrite(_ps2, LOW);
    digitalWrite(_osk, LOW);
    
    if(divider){
      _refClk = ref*divider;
    }else{
      _refClk = ref;
    }
    if (reset) {
        AD9910::reset();
    }
    
  delay(1);
  
  reg_t cfr2;
  cfr2.addr = 0x01;
  cfr2.data.bytes[0] = 0x02;
  cfr2.data.bytes[1] = 0x08;
  cfr2.data.bytes[2] = 0x00;  // sync_clk pin disabled; not used
  cfr2.data.bytes[3] = 0x01;  // enable ASF
    
    
    
    reg_t cfr3;
    cfr3.addr = 0x02;
    cfr3.data.bytes[0] = divider << 1; // pll divider
    if (divider == 0){
        cfr3.data.bytes[1] = 0x40;    // bypass pll
        cfr3.data.bytes[3] = 0x07;
    } else {
        cfr3.data.bytes[1] = 0x41;    // enable PLL
        cfr3.data.bytes[3] = 0x05;
    }
    cfr3.data.bytes[2] = 0x3F;
    
    
    
    writeRegister(cfr2);
    writeRegister(cfr3);    
    update();
    
    delay(1);
    
    _profileModeOn = false; //profile mode is disabled by default
    _OSKon = false; //OSK is disabled by default
    _activeProfile = 0; 
    

}

// reset() - takes no arguments; resets DDS
void AD9910::reset(){
    digitalWrite(_resetPin, HIGH);
    delay(1);
    digitalWrite(_resetPin, LOW);
}

// update() - sends a logic pulse to IO UPDATE pin on DDS; updates frequency output to 
//      newly set frequency (FTW0)
void AD9910::update(){
    digitalWrite(_updatePin, HIGH);
    delayMicroseconds(1);
    digitalWrite(_updatePin, LOW);
}

// setFreq(freq) -- writes freq to DDS board, in FTW0
void AD9910::setFreq(uint32_t freq, uint8_t profile){  // In this Eurocard implementation particular, we are only using profile 0 for each device
    
    if (profile > 7) {
        return; //invalid profile, return without doing anything
    } 
    if (freq>FREQ_UPPERLIM){
      freq = FREQ_UPPERLIM;
    }else if(freq<FREQ_LOWERLIM){
      freq = FREQ_LOWERLIM;
    }

    // set _freq and _ftw variables
    uint32_t amp=_amp[profile];
    uint32_t phase_offset=_phase_offset[profile];
    setWave(freq,phase_offset,amp,profile);
}

// setWave(): Set the wave output in the single profile mode of AD9910
void AD9910::setWave(uint32_t freq, uint32_t phase_offset, uint32_t amp, uint8_t profile){
    // uint32_t a =micros();
    if (profile > 7) {
        return; //invalid profile, return without doing anything
    } 
    if (freq>FREQ_UPPERLIM){
      freq = FREQ_UPPERLIM;
    }else if(freq<FREQ_LOWERLIM){
      freq = FREQ_LOWERLIM;
    }
    if (amp>AMP_UPPERLIM){
      amp = AMP_UPPERLIM;
    }else if(amp<AMP_LOWERLIM){
      amp = AMP_LOWERLIM;
    }

    _freq[profile] = freq;
    _ftw[profile] = round(freq * RESOLUTION / _refClk) ;

    _phase_offset[profile] = phase_offset;
    _pow[profile] = round(phase_offset * (65536-1) / 360) ;

    _amp[profile] = amp; 
    _asf[profile] = round(amp * (16384-1) / 100);

    reg_t payload;
    payload.bytes = 8;
    payload.addr = 0x0E + profile;
    payload.data.block[0] =  _ftw[profile];
    payload.data.block[1] = _asf[profile] * 65536 + _pow[profile];
    // Serial.println(micros()-a);Takes around 1 us per loop
    writeRegister(payload);
    update();
}


void AD9910::setAmp(uint32_t amp, uint8_t profile){
  // uint32_t a =micros();
  if (profile > 7) {
      return; //invalid profile, return without doing anything
  } 
  if (amp>AMP_UPPERLIM){
    amp = AMP_UPPERLIM;
  }else if(amp<AMP_LOWERLIM){
    amp = AMP_LOWERLIM;
  }
  uint32_t freq=_freq[profile];
  uint32_t phase_offset=_phase_offset[profile];
  setWave(freq,phase_offset,amp,profile);
}

//writeRegister() -- The command writes data to the register of AD9910; normally called by functions above
void AD9910::writeRegister(reg_t payload){
    // uint32_t a =micros();
    SPI.beginTransaction(SPISettings(CLOCKSPEED, MSBFIRST, SPI_MODE0));
    
    digitalWrite(_ssPin, LOW);
    SPI.transfer(payload.addr);
    // MSB
    for (int i = payload.bytes; i > 0; i--){
        SPI.transfer(payload.data.bytes[i-1]);
    }
    digitalWrite(_ssPin, HIGH);
    
    SPI.endTransaction();
    // Serial.println(micros()-a); // Takes around 3 us per loop
}

AD9910 DDS0(DDS0_CS, DDS0_RESET, DDS0_IOUPDATE, DDS0_PS0, DDS0_PS1, DDS0_PS2, DDS0_OSK);







/* Unfinished/Untested/Unused functions


void AD9910::setDR(uint32_t upperlimit, uint32_t lowlimit, uint32_t decreStep, uint32_t increStep,uint16_t negSlope,uint16_t posSlope){
    // set _freq and _ftw variables
    _freq[profile] = freq;
    _ftw[profile] = round(freq * RESOLUTION / _refClk) ;

    _phase_offset[profile] = phase_offset;
    _pow[profile] = round(phase_offset * 65536 / 360) ;

    _amp[profile] = amp;
    _asf[profile] = round(amp * 16384 / 100) ;

    reg_t payload;
    payload.bytes = 8;
    payload.addr = 0x0E + profile;
    payload.data.block[0] =  _ftw[profile];
    payload.data.block[1] = _asf[profile] * 65536 + _pow[profile];

	// actually writes to register
    //AD9910::writeRegister(CFR1Info, CFR1);
    writeRegister(payload);
    update();
}
*/
/*
void AD9910::setAmp(double scaledAmp, byte profile){
   if (profile > 7) {
        return; //invalid profile, return without doing anything
   } 
   
   _scaledAmp[profile] = scaledAmp;
   _asf[profile] = round(scaledAmp*4096);
   _scaledAmpdB[profile] = 20.0*log10(_asf[profile]/4096.0);
   
   if (_asf[profile] >= 4096) {
      _asf[profile]=4095; //write max value
   } else if (scaledAmp < 0) {
      _asf[profile]=0; //write min value
   }
   
   AD9910::writeAmp(_asf[profile],profile);

}
       
void AD9910::setAmp(double scaledAmp){
  AD9910::setAmp(scaledAmp,0);
}

void AD9910::setAmpdB(double scaledAmpdB, byte profile){
  if (profile > 7) {
        return; //invalid profile, return without doing anything
   } 
   
   if (scaledAmpdB > 0) {
       return; //only valid for attenuation, so dB should be less than 0, return without doing anything
   }
   
   _scaledAmpdB[profile] = scaledAmpdB;
   _asf[profile] = round(pow(10,scaledAmpdB/20.0)*4096.0);
   _scaledAmp[profile] = _asf[profile]/4096.0;
   
   if (_asf[profile] >= 4096) {
      _asf[profile]=4095; //write max value
   }
   
   AD9910::writeAmp(_asf[profile],profile);
   
}

void AD9910::setAmpdB(double scaledAmpdB){
  AD9910::setAmpdB(scaledAmpdB,0);
}

//Gets current amplitude
double AD9910::getAmp(byte profile){
  return _scaledAmp[profile];
}
double AD9910::getAmp(){
  return _scaledAmp[0];
}
        
// Gets current amplitude in dB
double AD9910::getAmpdB(byte profile){
  return _scaledAmpdB[profile];
}
double AD9910::getAmpdB(){
  return _scaledAmpdB[0];
}
        
//Gets current amplitude tuning word
unsigned long AD9910::getASF(byte profile){
  return _asf[profile];
}
unsigned long AD9910::getASF(){
  return _asf[0];
}



// getFreq() - returns current frequency
unsigned long AD9910::getFreq(byte profile){
    return _freq[profile];
}
// 
// // getFreq() - returns frequency from profile 0
// unsigned long AD9910::getFreq(){
//     return _freq[0];
// }

// getFTW() -- returns current FTW
unsigned long AD9910::getFTW(byte profile){
    return _ftw[profile];
}

// unsigned long AD9910::getFTW(){
//     return _ftw[0];
// }

// Function setFTW -- accepts 32-bit frequency tuning word ftw;
//      updates instance variables for FTW and Frequency, and writes ftw to DDS.
void AD9910::setFTW(unsigned long ftw, byte profile){
  
    if (profile > 7) {
        return; //invalid profile, return without doing anything
    } 
    
    // set freqency and ftw variables
    _ftw[profile] = ftw;
    _freq[profile] = ftw * _refClk / RESOLUTION;

    // divide up ftw into four bytes
    byte data[] = { lowByte(ftw >> 24), lowByte(ftw >> 16), lowByte(ftw >> 8), lowByte(ftw)};
    // register info -- writing four bytes to register 0x04, 

    byte registerInfo[] = {0x0B, 4};
    registerInfo[0] += 2*profile; //select the right register for the commanded profile number

	
    //byte CFR1[] = { 0x00, 0x00, 0x00, 0x00 };
    //byte CFR1Info[] = {0x00, 4};
	
    //AD9910::writeRegister(CFR1Info, CFR1);
    AD9910::writeRegister(registerInfo, data);
    AD9910::update();

}

// void AD9910::setFTW(unsigned long ftw){
//   AD9910::setFTW(ftw,0);
// }

//Enable the profile select mode
void AD9910::enableProfileMode() {
  //write 0x01, byte 23 high
  _profileModeOn = true;
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x80, 0x09, 0x00};
  AD9910::writeRegister(registerInfo, data);
  AD9910::update();
}

//Disable the profile select mode
void AD9910::disableProfileMode() {
  //write 0x01, byte 23 low
  _profileModeOn = false;
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x00, 0x09, 0x00};
  AD9910::writeRegister(registerInfo, data);
  AD9910::update();
}

//enable OSK
void AD9910::enableOSK(){
  //write 0x00, byte 8 high
  _OSKon = true;
  byte registerInfo[] = {0x00, 4};
  byte data[] = {0x00, 0x01, 0x01, 0x08};
  AD9910::writeRegister(registerInfo, data);
  AD9910::update();
}
        
//disable OSK
void AD9910::disableOSK(){
  //write 0x00, byte 8 low
  _OSKon = false;
  byte registerInfo[] = {0x00, 4};
  byte data[] = {0x00, 0x01, 0x00, 0x08};
  AD9910::writeRegister(registerInfo, data);
  AD9910::update();
}
  
//return boolean indicating if profile select mode is activated
boolean AD9910::getProfileSelectMode() {
  return _profileModeOn;
}

//return boolean indicating if OSK mode is activated
boolean AD9910::getOSKMode() {
  return _OSKon;
}

void AD9910::enableSyncClck() {
 //write 0x01, byte 11 high 
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x80, 0x09, 0x00};
  AD9910::writeRegister(registerInfo, data);
  AD9910::update();
}

void AD9910::disableSyncClck() {
  //write 0x01, byte 11 low
  byte registerInfo[] = {0x01, 4};
  byte data[] = {0x00, 0x80, 0x01, 0x00};
  AD9910::writeRegister(registerInfo, data);
  AD9910::update();
}

void AD9910::selectProfile(byte profile){
  //Possible improvement: write PS pin states all at once using register masks
  _activeProfile = profile;
  
  if (profile > 7) {
    return; //not a valid profile number, return without doing anything
  }
  
  if ((B00000001 & profile) > 0) { //rightmost bit is 1
      digitalWrite(_ps0, HIGH);
  } else {
      digitalWrite(_ps0,LOW);
  }
  if ((B00000010 & profile) > 0) { //next bit is 1
      digitalWrite(_ps1, HIGH);
  } else {
      digitalWrite(_ps1,LOW);
  }
  if ((B00000100 & profile) > 0) { //next bit is 1
      digitalWrite(_ps2, HIGH);
  } else {
      digitalWrite(_ps2,LOW);
  }
  
}

byte AD9910::getProfile() {
  return _activeProfile;
}
*/

// Writes SPI to particular register.
//      registerInfo is a 2-element array which contains [register, number of bytes]


// void AD9910::short_writeRegister(short_reg_t payload){
//     SPI.beginTransaction(SPISettings(CLOCKSPEED, MSBFIRST, SPI_MODE0));
    
//     digitalWrite(_ssPin, LOW);
//     SPI.transfer(payload.addr);
//     // MSB
//     for (int i = payload.bytes; i > 0; i--){
//         SPI.transfer(payload.data.bytes[i-1]);
//     }
//     digitalWrite(_ssPin, HIGH);
    
//     SPI.endTransaction();
// }

/* PRIVATE CLASS FUNCTIONS */


/*

void AD9910::writeAmp(long ampScaleFactor, byte profile){
  byte registerInfo[] = {0x0C, 4};
  
  registerInfo[0] += 2*profile; //select the right register for the commanded profile number
  
  // divide up ASF into two bytes, pad with 0s for the phase offset
  byte atw[] = {lowByte(ampScaleFactor >> 8), lowByte(ampScaleFactor), 0x00, 0x00};
  
  // actually writes to register
  AD9910::writeRegister(registerInfo, atw);
  
  AD9910::update();
  
}
*/
