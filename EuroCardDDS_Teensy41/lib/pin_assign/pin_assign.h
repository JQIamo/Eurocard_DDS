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


#define DDS0_RESET 		1
#define DDS0_CS 		10
#define DDS0_PS0 		3
#define DDS0_PS1 		4
#define DDS0_PS2 		6
#define DDS0_OSK 		2
#define DDS0_IOUPDATE 	5
#define DDS0_DROVER 	9
#define DDS0_DRCTL 	    8
#define DDS0_DRHOLD 	7
#define DDS0_MOSI		11
#define DDS0_CLK		13

#define PLL_LCK 	    23

//  \name Pins for LCD & Switches/encoders

#define LCD_RST			12
#define LCD_RS			14
#define LCD_CS			0
#define LCD_MOSI		26
#define LCD_CLK			27

#define ENC_A			36
#define ENC_B			35
#define ENC_SW			34


//  \name Pins for SPI & SetList triggering


#define SETLIST_TRIG	33
#define ANALOG_AMP	    11 // This is the index of analog pin, not the real pin index
#define ANALOG_FREQ	    10 // This is the index of analog pin, not the real pin index

//  \name Pins for inter-board communication
#define SERIAL_TX	    20
#define SERIAL_RX   	21

//! @}
