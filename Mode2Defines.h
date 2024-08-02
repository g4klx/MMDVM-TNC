/*
 *   Copyright (C) 2023 by Jonathan Naylor G4KLX
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


#if !defined(MODE2DEFINES_H)
#define  MODE2DEFINES_H

#include <cstdint>

const uint8_t MODE2_RADIO_SYMBOL_LENGTH = 5U;      // At 24 kHz sample rate

const uint8_t MODE2_PREAMBLE_BYTE = 0x77U;

const uint8_t MODE2_SYMBOLS_PER_BYTE = 4U;

const uint8_t  MODE2_HEADER_PARITY_BYTES   = 2U;
const uint8_t  MODE2_HEADER_PARITY_SYMBOLS = MODE2_HEADER_PARITY_BYTES * MODE2_SYMBOLS_PER_BYTE;
const uint16_t MODE2_HEADER_PARITY_SAMPLES = MODE2_HEADER_PARITY_SYMBOLS * MODE2_RADIO_SYMBOL_LENGTH;

const uint8_t  MODE2_PAYLOAD_PARITY_BYTES   = 16U;
const uint8_t  MODE2_PAYLOAD_PARITY_SYMBOLS = MODE2_PAYLOAD_PARITY_BYTES * MODE2_SYMBOLS_PER_BYTE;
const uint16_t MODE2_PAYLOAD_PARITY_SAMPLES = MODE2_PAYLOAD_PARITY_SYMBOLS * MODE2_RADIO_SYMBOL_LENGTH;

const uint8_t  MODE2_CRC_LENGTH_BYTES     = 4U;
const uint8_t  MODE2_CRC_LENGTH_SYMBOLS   = MODE2_CRC_LENGTH_BYTES * MODE2_SYMBOLS_PER_BYTE;
const uint16_t MODE2_CRC_LENGTH_SAMPLES   = MODE2_CRC_LENGTH_SYMBOLS * MODE2_RADIO_SYMBOL_LENGTH;

const uint8_t  MODE2_SYNC_LENGTH_BYTES   = 4U;
const uint8_t  MODE2_SYNC_LENGTH_SYMBOLS = MODE2_SYNC_LENGTH_BYTES * MODE2_SYMBOLS_PER_BYTE;
const uint16_t MODE2_SYNC_LENGTH_SAMPLES = MODE2_SYNC_LENGTH_SYMBOLS * MODE2_RADIO_SYMBOL_LENGTH;
const uint8_t  MODE2_SYNC_BYTES[]        = { 0x5D, 0x57U, 0xDFU, 0x7FU};

const uint8_t  MODE2_HEADER_LENGTH_BYTES   = 13U;
const uint8_t  MODE2_HEADER_LENGTH_SYMBOLS = MODE2_HEADER_LENGTH_BYTES * MODE2_SYMBOLS_PER_BYTE;
const uint16_t MODE2_HEADER_LENGTH_SAMPLES = MODE2_HEADER_LENGTH_SYMBOLS * MODE2_RADIO_SYMBOL_LENGTH;

// 5     D      5     7      D     F      7     F
// 01 01 11 01  01 01 01 11  11 01 11 11  01 11 11 11
// +3 +3 -3 +3  +3 +3 +3 -3  -3 +3 -3 -3  +3 -3 -3 -3

const int8_t MODE2_SYNC_SYMBOLS_VALUES[] = { +3, +3, -3, +3,    +3, +3, +3, -3,   -3, +3, -3, -3,  +3, -3, -3, -3};

const uint16_t MODE2_SYNC_SYMBOLS = 0xDE48U;

#endif

