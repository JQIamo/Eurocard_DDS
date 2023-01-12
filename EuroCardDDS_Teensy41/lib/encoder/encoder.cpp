// The encoder used for DDS with an analog control by Juntian Tu at JQI in Jan. 2023
// Inherited and editted from https://lastminuteengineers.com/rotary-encoder-arduino-tutorial/

#include "Arduino.h"
#include "SPI.h"
#include "pin_assign.h"
#include "encoder.h"
// Rotary Encoder Inputs
void Encoder::setup() {
	
	// Set encoder pins as inputs
	pinMode(ENC_A,INPUT);
	pinMode(ENC_B,INPUT);
	pinMode(ENC_SW, INPUT_PULLUP);

	// Read the initial state of ENC_A
	lastStateENC_A = digitalRead(ENC_A);
}

char Encoder::reader(){
    currentStateENC_A = digitalRead(ENC_A);
	
	// If last and current state of ENC_A are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentStateENC_A != lastStateENC_A  && currentStateENC_A == 1){
		unsigned long currenttime = millis();
		// Here -,m,M and +,p,P indicates the different speed of rotation at two directions
		if (digitalRead(ENC_B) == currentStateENC_A) {
			if (millis() - lastRotation < 200){
				lastRotation = currenttime;
				timescounter ++;
				if (timescounter >= 4){
					timescounter = 0;
					if (lastreturned == '-'){
						lastreturned = 'm';
						return 'm';
					}else if (lastreturned == 'm'){
						return 'M';
					}else{
						lastreturned = '-';
						return '-';
					}}
				else if (lastreturned == '-' or lastreturned == 'm'){
					return lastreturned;				
				}
			}
			lastRotation = currenttime;
			lastreturned = '-';
			timescounter = 0;
			return '-';
		} else {
			if (millis() - lastRotation < 200){
				timescounter ++;
				lastRotation = currenttime;
				if (timescounter >= 4){
					timescounter = 0;
					if (lastreturned == '+'){
						lastreturned = 'p';
						return 'p';
					}else if (lastreturned == 'p'){
						return 'P';
					}else{
						lastreturned = '+';
						return '+';
					}}
				else if (lastreturned == '+' or lastreturned == 'p'){
					return lastreturned;				
				}
			}
			lastRotation = currenttime;
			lastreturned = '+';
			timescounter = 0;
			return '+';
		}
	}

	// Remember last ENC_A state
	lastStateENC_A = currentStateENC_A;

	// Read the button state
	int btnState = digitalRead(ENC_SW);

	//If we detect LOW signal, button is pressed
	if (btnState == LOW) {
		//if 50ms have passed since last LOW pulse, it means that the
		//button has been pressed, released and pressed again
		delay(50);
		while (digitalRead(ENC_SW) == LOW){}
		if (millis() - lastButtonPress > 100) { // This line added to prevent bouncing back of the button
			lastButtonPress = millis();
			return 'c';
		}
	}
	return 0;
}

Encoder encoder;