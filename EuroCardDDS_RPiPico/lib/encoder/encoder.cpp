// The encoder used for DDS with an analog control by Oliver/Juntian Tu at JQI in May. 2023
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
			if (currenttime - lastRotation > 100){
				if (currenttime - lastRotation < 250){
					lastRotation = currenttime;
					timescounter ++;
					if (timescounter >= 3){ // Moving at same direction in high speed for 3 times brings to a higher mode
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
			}
		} else {
			if (currenttime - lastRotation > 100){
				if (currenttime - lastRotation < 250){
					timescounter ++;
					lastRotation = currenttime;
					if (timescounter >= 3){
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
	}

	// Remember last ENC_A state
	lastStateENC_A = currentStateENC_A;

	// Read the button state
	int btnState = digitalRead(ENC_SW);

	//If we detect LOW signal, button is pressed
	if (btnState == LOW) {
		// if 50ms have passed since last LOW pulse, it means that the
		// button has been pressed, released and pressed again
		delay(50);
		unsigned long pressed_moment = millis();
		while (digitalRead(ENC_SW) == LOW){}
		if (pressed_moment - lastButtonPress > 100){ // This condition added to prevent bouncing back of the button
			unsigned long released_moment = millis();
			if (released_moment - pressed_moment > 300) {
				lastButtonPress = released_moment;
				return 'h';
			}
			else {
				lastButtonPress = released_moment;
				return 'c';
			}
		}
	}
	return 0;
}

Encoder encoder;