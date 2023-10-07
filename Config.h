/*
 *   Copyright (C) 2015,2016,2017,2018,2020,2023 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#if !defined(CONFIG_H)
#define  CONFIG_H

// Allow for the use of high quality external clock oscillators
// The number is the frequency of the oscillator in Hertz.
//
// The frequency of the TCXO must be an integer multiple of 48000.
// Frequencies such as 12.0 Mhz (48000 * 250) and 14.4 Mhz (48000 * 300) are suitable.
// Frequencies such as 10.0 Mhz (48000 * 208.333) or 20 Mhz (48000 * 416.666) are not suitable.
//
// For 12 MHz
#define EXTERNAL_OSC 12000000
// For 12.288 MHz
// #define EXTERNAL_OSC 12288000
// For 14.4 MHz
// #define EXTERNAL_OSC 14400000
// For 19.2 MHz
// #define EXTERNAL_OSC 19200000

// Baud rate for host communication.
#define SERIAL_SPEED	115200

// Select the initial packet mode
// 1 = 1200 bps AFSK AX.25
// 2 = 9600 bps C4FSK IL2P
#define	INITIAL_MODE	2

// TX Delay in milliseconds
#define	TX_DELAY	300

// TX Tail in milliseconds
#define	TX_TAIL		50

// P-Persistence in x/255
#define	P_PERSISTENCE	63

// Slot Time in milliseconds
#define	SLOT_TIME	100

// Set Duplex, 1 for full duplex, 0 for simplex
#define	DUPLEX		0

// Select use of serial debugging
#define	SERIAL_DEBUGGING

// Baud rate for serial debugging.
#define DEBUGGING_SPEED	38400

// Set the receive level (out of 255)
#define	RX_LEVEL	128

// Set the mode 1 transmit level (out of 255)
#define	MODE1_TX_LEVEL	128

// Set the mode 2 transmit level (out of 255)
#define	MODE2_TX_LEVEL	128

// Use pins to output the current mode via LEDs
#define MODE_LEDS

// For the original Arduino Due pin layout
// #define ARDUINO_DUE_PAPA

#if defined(STM32F1)
// For the SQ6POG board
#define STM32F1_POG
#else
// For the ZUM V1.0 and V1.0.1 boards pin layout
// #define ARDUINO_DUE_ZUM_V10
#endif

// For the SP8NTH board
// #define ARDUINO_DUE_NTH

// For ST Nucleo-64 STM32F446RE board
// #define STM32F4_NUCLEO_MORPHO_HEADER
// #define STM32F4_NUCLEO_ARDUINO_HEADER

// For the VK6MST Pi3 Shield communicating over i2c. i2c address & speed defined in i2cTeensy.cpp
// #define VK6MST_TEENSY_PI3_SHIELD_I2C

// Constant Service LED once the TNC is running 
// Do not use if employing an external hardware watchdog 
// #define CONSTANT_SRV_LED

#endif

