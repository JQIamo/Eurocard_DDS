/*!	\file	pin_assign.h
 *	\brief	Pin assignment of a Teensy 4.1 for the DDS on Eurocard module.
 *	\details It is compatible with the DDS on Eurocard.
 *          See https://github.com/TGOJName/DDS-Controller for detail.
 *          Thanks to Neal Pisent for his code as a reference.
 *
 *	\author	Juntian Tu <juntian@umd.edu>
 *	\date	2022-12
 */


// \name Pins for DDS-type objects
// For RPi Pico, the pin index is the index of corresponding GPIO pin

#define DDS0_RESET 		18u
#define DDS0_CS 		13u
#define DDS0_PS0 		14u
#define DDS0_PS1 		12u
#define DDS0_PS2 		8u
#define DDS0_OSK 		15u
#define DDS0_IOUPDATE 	9u
#define DDS0_DROVER 	5u
#define DDS0_DRCTL 	    6u
#define DDS0_DRHOLD 	7u
#define DDS0_MOSI		11u
#define DDS0_CLK		10u

#define PLL_LCK 	    4u

//  \name Pins for LCD & Switches/encoders

#define LCD_RST			20u
#define LCD_RS			19u
#define LCD_CS			17u
#define LCD_MOSI		3u
#define LCD_CLK			2u

#define ENC_A			21u
#define ENC_B			22u
#define ENC_SW			27u


//  \name Pins for SPI & SetList triggering


#define SETLIST_TRIG	0u
#define ANALOG_AMP	    A2 // This is the index of analog pin, not the real pin index
#define ANALOG_FREQ	    A0 // This is the index of analog pin, not the real pin index

//  \name Pins for inter-board communication
#define SERIAL_TX	    16u // Represented as Serial1
#define SERIAL_RX   	1u

//! @}
