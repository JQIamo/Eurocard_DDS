// SetListArduino library modified for DDS with an analog control by Oliver/Juntian Tu at JQI in May. 2023
// Inherited and editted from https://github.com/JQIamo/SetListArduino-arduino written by Neal Pisenti at JQI

/*
   SetListArduino.cpp - Arduino Integration for SetList computer control
   Created by Neal Pisenti, 2014
   JQI - Strontium - UMD
   
   SetList is a labview-based computer control suite in use at the JQI.
   Code and more info can be found on GitHub: 
   https://www.github.com/JQIamo/SetList

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

#include "SetListArduino.h"
#include "menus.h"

/***********************************************
	SetListBase Class definitions
************************************************/

// No need to do anything here, simply a container for SetListDevices.
// By Oliver: Sometime 
SetListBase::SetListBase(){;}   
void SetListBase::insertToSetList(int pos, GenericSetListCallback func, int * params){;}
void SetListBase::executeSetList(int pos){;}
int  SetListBase::getSetListFunc(int pos){;}
int * SetListBase::getSetListParams(int pos){;}
int SetListBase::getSetListLength(){;}
void SetListBase::clearSetList(){;}


/***********************************************
    SetListArduino Class Definitions
************************************************/

// SetListArduino() -- Constructor for SetListArduino class.
//      Initializes some private vairables, etc.
SetListArduino::SetListArduino(int triggerChannel) :
    _deviceCount(0),
    _activeDevice(0),
    _serialTerm('\n'),
    _last(NULL),
    _line(0),
    _errorFlag(0)
{
    _triggerChannel = triggerChannel;
    
    // configure some stuff for the serial parsing...
    strcpy(_delim, " ");	// set string delimiter for strtok_r
    clearSerialBuffer();	// clear serial buffer and prepare to readSerial
	
	//detachInterrupt(_triggerChannel);
	attachInterrupt(_triggerChannel, SetListISR::dummyInterrupt, FALLING);
}



// triggerUpdate() - execute next line in setlist.
void SetListArduino::triggerUpdate(){
    for (int i = 0; i < _deviceCount; i++){
        _deviceList[i]->executeSetList(_line); 
    }
    _line++;
}

// clearSetList() - resets length and line counters
void SetListArduino::clearSetList(){
    _line = 0;
    _setlistLength = 0;
}

// getTriggerChannel() - returns trigger channel for SetListArduino
int SetListArduino::getTriggerChannel(){
    return _triggerChannel;
}

// readSerial() -- function to parse serial stream, look for commands, and 
//      add the appropriate callbacks to the appropriate device's _setlist.
// 		This is based heavily off of kroimon's SerialCommand library:
//      https://github.com/kroimon/Arduino-SerialCommand
void SetListArduino::readSerial(){  // MODIFIED
	// initialize error flag to false; will throw if encounters issue.
	// MODIFIED {
	_errorFlag = false;
	bool ignore = true; // MODIFIED: Since we are using different modules with single DDS on each module,
						//	 we use channel index to distinguish the modules, and the command of other
						// 	 channels should be ignored indicated by this ignore flag
	bool run = false;
	passer_tracker=0;
	_char_passer[0] = '\0'; // Used for passing the received content to other modules
    while (Serial.available() > 0){
        // read in next character from serial stream
        char inChar = Serial.read(); // MODIFIED
		// Serial.println(inChar);
		if(inChar == '^'){Serial.println('O');return;} // MODIFIED: Added for connection check
		_char_passer[passer_tracker] = inChar;  // MODIFIED: Recording the serial content for passing to other microcontroller later
		_char_passer[passer_tracker+1] = '\0';  // MODIFIED
		passer_tracker++;  // MODIFIED
        
        if (inChar == _serialTerm) {
            // reached serial line terminator, so tokenize string
            char * command = strtok_r(_buffer, _delim, & _last);
            char * param;
            
            // check to see if command was a "special" command
            bool specialCmd = false;
            switch (*command) {
            	case _activateDeviceCmd:
            		specialCmd = true;
            		_line = 0;
                	_setlistLength = 0; // reset counter for setlist...
                                    	// all of the logic here assumes you're
                                    	// sending setlists of equal length for 
                                    	// all devices!!
            		
            		param = strtok_r(NULL, _delim, & _last);
            		
            		#ifdef SETLIST_DEBUG
            			Serial.print("Activating device: ");
            			Serial.println(param);
            		#endif
            		
            		// Parse device number, error check, and activate device
            		if (param != NULL) {

						if (atoi(param) == channel_index){

						ignore = false;
            			
                    	int channel = 0;
                    	              
						if(channel >= 0 && channel < _deviceCount){
							_activeDevice = channel;
							_deviceList[_activeDevice]->clearSetList();
						} else {
							#ifdef SETLIST_ERROR_CHECK
								Serial.println("ArduinoError: Invalid Channel");
							#endif
						}
						} else {
							ignore = true;
						}
					} else {
						#ifdef SETLIST_ERROR_CHECK
							Serial.println("ArduinoError: Invalid Param");
						#endif                  
					}      
					
            		break;
            		
            	case _initRunCmd:
            		specialCmd = true;
					run = true;
            		_line = 0;
                	            		
            		
            		// check that _setlist table is rectangular...
            		for (int d = 0; d < _deviceCount; d++){
            			if(_deviceList[d]->getSetListLength() != _setlistLength)
            			{	
            				_errorFlag = true;
            				break;
            			}
            		}
            		
            				
            		if (!_errorFlag){
						#ifdef SETLIST_DEBUG
							Serial.println("Init SetList run...");
						#endif
						
						// reset ISR
						detachInterrupt(_triggerChannel);
						attachInterrupt(_triggerChannel,
							SetListISR::firstTriggerInterrupt, FALLING);
						
						// initialize by executing first setlist line
						
						triggerUpdate();
							
					}
            		break;
            		
            	case _echoSetListCmd:
					if(! ignore){
            		specialCmd = true;
            		
            		#ifdef SETLIST_DEBUG
            			Serial.println("Here is the programmed setlist:");
            		#endif
            		
            		for (int i = 0; i < _deviceCount; i++){
            			Serial.print("Device #");
            			Serial.println(i);
            			Serial.print("Setlist lines: ");
            			int setlistLength = _deviceList[i]->getSetListLength();
            			Serial.println(setlistLength);
            			for (int j = 0; j < setlistLength; j++){
            				Serial.print("ln ");
            				Serial.print(j);
            				Serial.print("; Callback Ptr ");
            				Serial.print(_deviceList[i]->getSetListFunc(j));
            				Serial.print("; Params ");
            				int * lineParams = 
            					_deviceList[i]->getSetListParams(j);
            				for(int k = 0; k < MAX_PARAM_NUM; k++){
            					Serial.print(lineParams[k]);
            					Serial.print(" ");
            				}
            				Serial.println(";");
            			}
            			Serial.println("----------");
            			if(setlistLength != _setlistLength){
            				Serial.print("There is a mismatch in setlist lines. Device thinks there are ");
            				Serial.print(setlistLength);
            				Serial.print(" lines, while SetListImage thinks there are ");
            				Serial.print(_setlistLength);
            				Serial.println(" lines. Get yo' life together!");
            			}
            		}
					}
            		break;
            	
            	case _executeSingleLine:
					if (!ignore){
            		specialCmd = true;
            		int ch = atoi(strtok_r(NULL, _delim, & _last));
            		int ln = atoi(strtok_r(NULL, _delim, & _last));
            		#ifdef SETLIST_DEBUG
            			Serial.println("Executing single line...");
            			Serial.print("Channel: ");
            			Serial.print(ch);
            			Serial.print(" Line: ");
            			Serial.println(ln);
            		#endif
            		if ( ch >= 0 && ch < _deviceCount){
            			_deviceList[ch]->executeSetList(ln);
            		} else {
            			#ifdef SETLIST_DEBUG
            				Serial.print("Channel out of range. Only ");
            				Serial.print(_deviceCount);
            				Serial.println(" devices are registered.");
            			#endif
            		}}
            		break;	
            }
			
			
            
            // If command isn't one of the "special" commands,
        	// try to match it with list of registered commands.
            if (command != NULL && !specialCmd) {
                
                boolean matched = false;
                for (int i = 0; i < _commandCount; i++) {
                    #ifdef SETLIST_DEBUG
                    	Serial.print("Trying to match command: ");
                    	Serial.println(_commandList[i].command);
                    #endif
                    
                    // check to see if command is in commandList,
                    // and channel matches with currently activated device.
                    if ((strncmp(command, _commandList[i].command, 
                            SERIALCOMMAND_MAXCOMMANDLENGTH) == 0) &&
                            (_commandList[i].channel == _activeDevice)){
                        
                        #ifdef SETLIST_DEBUG
                        	Serial.print("Matched command: ");
                        	Serial.print(_commandList[i].command);
                        	Serial.print(", Channel #: ");
                        	nerial.println(_activeDevice);
							Serial.println("These are the params: ");
                        #endif
                        
                        int paramList[MAX_PARAM_NUM];

                        for (int p = 0; p < MAX_PARAM_NUM; p++){
                        	char * paramChar = strtok_r(NULL, _delim, & _last);
                        	if (paramChar != NULL){
								paramList[p] = atoi(paramChar);
                            	#ifdef SETLIST_DEBUG
									Serial.println(paramChar);
									Serial.println(atoi(paramChar));
								#endif
                            } else {
                            	#ifdef SETLIST_DEBUG
									Serial.print("param was null: ");
									Serial.println(p);
                            	#endif
								paramList[p] = 0;
                            }
                        }
                        
                        #ifdef SETLIST_DEBUG
                        	Serial.print("Parameters passed: ");
                        	for(int p = 0; p < MAX_PARAM_NUM; p++){
                        		Serial.print(paramList[p]);
                        		Serial.print(", ");
                        	}
                        	Serial.println("");
                        	Serial.print("Inserting into setlist line #: ");
                        	Serial.println(_line);
                        #endif
                        
                        _deviceList[_activeDevice]->insertToSetList(_line++,
                					_commandList[i].function, paramList);
               			
               			_setlistLength++;	// increment setlist length counter
                        
                        matched = true;

                        
                        break;
                    }
                }   // end loop over _commandList
                
                // If command not matched, tell labview if ERR_CHECK defined.
                if (!matched) {
                    #ifdef SETLIST_ERROR_CHECK
                    	_errorFlag = true;
                    #endif
                    #ifdef SETLIST_DEBUG
                    	Serial.print("That was an invalid command. You sent ");
                    	Serial.print("(");
                    	Serial.print(_activeDevice);
                    	Serial.print(",");
                    	Serial.print(command);
                    	Serial.println("), but valid commands (channel, cmd) are:");
                    	for (int i = 0; i < _commandCount; i++){
                    		Serial.print("(");
                    		Serial.print(_commandList[i].channel);
                    		Serial.print(",");
                    		Serial.print(_commandList[i].command);
                    		Serial.print(")");
                    	}
                    	Serial.println("");
                    #endif
                    
                }
            }
            
            clearSerialBuffer();    // clear out serial buffer
                                    // to prepare for next line.
			if (_errorFlag){
				Serial.println("B");
			} else {
				if (run) {
				Serial.println("GR");	// OK status to labview
				} else {
				Serial.println("G");
				}
			}
        } 
        // If inChar isn't the serial line terminator, 
        // just add it to buffer and repeat.
        // if _bufPos > serial command buffer length, currently loose chars.
        // We will want to handle this properly at some point.
        else if(isprint(inChar)) {  // only add printable chars into buffer
            if (_bufPos < SERIALCOMMAND_BUFFER) {
                _buffer[_bufPos++] = inChar; // add char to buffer
                _buffer[_bufPos] = '\0';     // make sure null-terminated
            }
        }
    }   // end while(Serial.available());
}   // end readSerial();

void SetListArduino::readSerialH(int i){  // MODIFIED
	// initialize error flag to false; will throw if encounters issue.
	// MODIFIED {
	switch (i){
		case 1:
			nSerial = &Serial1;
			break;
		case 2:
			nSerial = &Serial2;
			break;
		case 3:
			nSerial = &Serial3;
			break;
		case 4:
			nSerial = &Serial4;
			break;
		case 5:
			nSerial = &Serial5;
			break;
		case 6:
			nSerial = &Serial6;
			break;
		case 7:
			nSerial = &Serial7;
			break;
		case 8:
			nSerial = &Serial8;
			break;
	}
	// if (serial_port_recorder[i-1] == '0'){
	// 	nSerial->begin(115200);
	// 	serial_port_recorder[i-1] = '1';
	// 	}
	// MODIFIED with all following Serial replaced by nSerial }
	_errorFlag = false;
	bool ignore = true; // MODIFIED: Since we are using different modules with single DDS on each module,
						//	 we use channel index to distinguish the modules, and the command of other
						// 	 channels should be ignored indicated by this ignore flag
	bool run = false;
	passer_tracker=0;
	_char_passer[0] = '\0'; // Used for passing the received content to other modules
	// Serial.println(nSerial);
    while (nSerial->available() > 0){
		// Serial.println(nSerial->available());
        // read in next character from serial stream
        char inChar = nSerial->read(); // MODIFIED
		// Serial.println(inChar);
		_char_passer[passer_tracker] = inChar;  // MODIFIED: Recording the serial content for passing to other microcontroller later
		_char_passer[passer_tracker+1] = '\0';  // MODIFIED
		passer_tracker++;  // MODIFIED
        
        if (inChar == _serialTerm) {
            // reached serial line terminator, so tokenize string
            char * command = strtok_r(_buffer, _delim, & _last);
            char * param;
            
            // check to see if command was a "special" command
            bool specialCmd = false;
            switch (*command) {
            	case _activateDeviceCmd:
            		specialCmd = true;
            		_line = 0;
                	_setlistLength = 0; // reset counter for setlist...
                                    	// all of the logic here assumes you're
                                    	// sending setlists of equal length for 
                                    	// all devices!!
            		
            		param = strtok_r(NULL, _delim, & _last);
            		
            		#ifdef SETLIST_DEBUG
            			Serial.print("Activating device: ");
            			Serial.println(param);
            		#endif
            		
            		// Parse device number, error check, and activate device
            		if (param != NULL) {

						if (atoi(param) == channel_index){

						ignore = false;
            			
                    	int channel = 0;
                    	              
						if(channel >= 0 && channel < _deviceCount){
							_activeDevice = channel;
							_deviceList[_activeDevice]->clearSetList();
						} else {
							#ifdef SETLIST_ERROR_CHECK
								Serial.println("ArduinoError: Invalid Channel");
							#endif
						}
						} else {
							ignore = true;
						}
					} else {
						#ifdef SETLIST_ERROR_CHECK
							Serial.println("ArduinoError: Invalid Param");
						#endif                  
					}      
					
            		break;
            		
            	case _initRunCmd:
            		specialCmd = true;
					run = true;
            		_line = 0;
                	            		
            		
            		// check that _setlist table is rectangular...
            		for (int d = 0; d < _deviceCount; d++){
            			if(_deviceList[d]->getSetListLength() != _setlistLength)
            			{	
            				_errorFlag = true;
            				break;
            			}
            		}
            		
            				
            		if (!_errorFlag){
						#ifdef SETLIST_DEBUG
							Serial.println("Init SetList run...");
						#endif
						
						// reset ISR
						detachInterrupt(_triggerChannel);
						attachInterrupt(_triggerChannel,
							SetListISR::firstTriggerInterrupt, FALLING);
						
						// initialize by executing first setlist line
						
						triggerUpdate();
							
					}
            		break;
            		
            	case _echoSetListCmd:
					if(! ignore){
            		specialCmd = true;
            		
            		#ifdef SETLIST_DEBUG
            			Serial.println("Here is the programmed setlist:");
            		#endif
            		
            		for (int i = 0; i < _deviceCount; i++){
            			Serial.print("Device #");
            			Serial.println(i);
            			Serial.print("Setlist lines: ");
            			int setlistLength = _deviceList[i]->getSetListLength();
            			Serial.println(setlistLength);
            			for (int j = 0; j < setlistLength; j++){
            				Serial.print("ln ");
            				Serial.print(j);
            				Serial.print("; Callback Ptr ");
            				Serial.print(_deviceList[i]->getSetListFunc(j));
            				Serial.print("; Params ");
            				int * lineParams = 
            					_deviceList[i]->getSetListParams(j);
            				for(int k = 0; k < MAX_PARAM_NUM; k++){
            					Serial.print(lineParams[k]);
            					Serial.print(" ");
            				}
            				Serial.println(";");
            			}
            			Serial.println("----------");
            			if(setlistLength != _setlistLength){
            				Serial.print("There is a mismatch in setlist lines. Device thinks there are ");
            				Serial.print(setlistLength);
            				Serial.print(" lines, while SetListImage thinks there are ");
            				Serial.print(_setlistLength);
            				Serial.println(" lines. Get yo' life together!");
            			}
            		}
					}
            		break;
            	
            	case _executeSingleLine:
					if (!ignore){
            		specialCmd = true;
            		int ch = atoi(strtok_r(NULL, _delim, & _last));
            		int ln = atoi(strtok_r(NULL, _delim, & _last));
            		#ifdef SETLIST_DEBUG
            			Serial.println("Executing single line...");
            			Serial.print("Channel: ");
            			Serial.print(ch);
            			Serial.print(" Line: ");
            			Serial.println(ln);
            		#endif
            		if ( ch >= 0 && ch < _deviceCount){
            			_deviceList[ch]->executeSetList(ln);
            		} else {
            			#ifdef SETLIST_DEBUG
            				Serial.print("Channel out of range. Only ");
            				Serial.print(_deviceCount);
            				Serial.println(" devices are registered.");
            			#endif
            		}}
            		break;	
            }
			
			
            
            // If command isn't one of the "special" commands,
        	// try to match it with list of registered commands.
            if (command != NULL && !specialCmd) {
                
                boolean matched = false;
                for (int i = 0; i < _commandCount; i++) {
                    #ifdef SETLIST_DEBUG
                    	Serial.print("Trying to match command: ");
                    	Serial.println(_commandList[i].command);
                    #endif
                    
                    // check to see if command is in commandList,
                    // and channel matches with currently activated device.
                    if ((strncmp(command, _commandList[i].command, 
                            SERIALCOMMAND_MAXCOMMANDLENGTH) == 0) &&
                            (_commandList[i].channel == _activeDevice)){
                        
                        #ifdef SETLIST_DEBUG
                        	Serial.print("Matched command: ");
                        	Serial.print(_commandList[i].command);
                        	Serial.print(", Channel #: ");
                        	nerial.println(_activeDevice);
							Serial.println("These are the params: ");
                        #endif
                        
                        int paramList[MAX_PARAM_NUM];

                        for (int p = 0; p < MAX_PARAM_NUM; p++){
                        	char * paramChar = strtok_r(NULL, _delim, & _last);
                        	if (paramChar != NULL){
								paramList[p] = atoi(paramChar);
                            	#ifdef SETLIST_DEBUG
									Serial.println(paramChar);
									Serial.println(atoi(paramChar));
								#endif
                            } else {
                            	#ifdef SETLIST_DEBUG
									Serial.print("param was null: ");
									Serial.println(p);
                            	#endif
								paramList[p] = 0;
                            }
                        }
                        
                        #ifdef SETLIST_DEBUG
                        	Serial.print("Parameters passed: ");
                        	for(int p = 0; p < MAX_PARAM_NUM; p++){
                        		Serial.print(paramList[p]);
                        		Serial.print(", ");
                        	}
                        	Serial.println("");
                        	Serial.print("Inserting into setlist line #: ");
                        	Serial.println(_line);
                        #endif
                        
                        _deviceList[_activeDevice]->insertToSetList(_line++,
                					_commandList[i].function, paramList);
               			
               			_setlistLength++;	// increment setlist length counter
                        
                        matched = true;

                        
                        break;
                    }
                }   // end loop over _commandList
                
                // If command not matched, tell labview if ERR_CHECK defined.
                if (!matched) {
                    #ifdef SETLIST_ERROR_CHECK
                    	_errorFlag = true;
                    #endif
                    #ifdef SETLIST_DEBUG
                    	Serial.print("That was an invalid command. You sent ");
                    	Serial.print("(");
                    	Serial.print(_activeDevice);
                    	Serial.print(",");
                    	Serial.print(command);
                    	Serial.println("), but valid commands (channel, cmd) are:");
                    	for (int i = 0; i < _commandCount; i++){
                    		Serial.print("(");
                    		Serial.print(_commandList[i].channel);
                    		Serial.print(",");
                    		Serial.print(_commandList[i].command);
                    		Serial.print(")");
                    	}
                    	Serial.println("");
                    #endif
                    
                }
            }
            
            clearSerialBuffer();    // clear out serial buffer
                                    // to prepare for next line.
			if (_errorFlag){
				Serial.println("B");
			} else {
				if (run) {
				Serial.println("GR");	// OK status to labview
				} else {
				Serial.println("G");
				}
			}
        } 
        // If inChar isn't the serial line terminator, 
        // just add it to buffer and repeat.
        // if _bufPos > serial command buffer length, currently loose chars.
        // We will want to handle this properly at some point.
        else if(isprint(inChar)) {  // only add printable chars into buffer
            if (_bufPos < SERIALCOMMAND_BUFFER) {
                _buffer[_bufPos++] = inChar; // add char to buffer
                _buffer[_bufPos] = '\0';     // make sure null-terminated
            }
        }
    }   // end while(nSerial->available());
}   // end readSerial();

char * SetListArduino::get_buffer(){  // MODIFIED: Used to retrieve the received content
	return _char_passer;
}

void SetListArduino::clearSerialBuffer(){

    _bufPos = 0;
    _buffer[0] = '\0';
}



/***********************************************
    SetListISR Class Definitions
************************************************/

// see note in SetListArduino.h as to why we are doing this...

//! Global singleton instance of SetListArduino.
/*!
	This is what you'll want to use in your Arduino sketches, rather than
	some custom-named instance of SetListArduino. See the 
	[README](md__r_e_a_d_m_e.html) for details on implementing this in 
	a sketch, and SetListISR for an explanation of why we are declaring this.
*/
extern SetListArduino SetListImage;

void SetListISR::firstTriggerInterrupt(){
	is_loop = 0;
	SetListImage.triggerUpdate();
    
    // switch interrupt to trigger on CHANGE for rest of trigger pulses
    int triggerChannel = SetListImage.getTriggerChannel();
    detachInterrupt(triggerChannel);
    attachInterrupt(triggerChannel, SetListISR::restTriggerInterrupt, CHANGE);
}

void SetListISR::restTriggerInterrupt(){
	is_loop = 0;
    SetListImage.triggerUpdate();
}

//! Dummy interrupt routine.
/*!
	This dummy interrupt routine is attached when SetListArduino object is declared.
	It does nothing, but fixes an irritating issue where SetListISR::firstTriggerInterrupt
	is executed when it is attached immediately after the Arduino resets. This should
	be ultimately fixed by clearing the ISR flags, but I'm not clear on the best
	way to do that in a platform-agnostic manner, since the Due allows interrupts to be 
	attached on any pin.
*/
void SetListISR::dummyInterrupt(){
}