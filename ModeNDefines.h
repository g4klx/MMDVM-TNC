/*
 *   Copyright (C) 2023,2024 by Jonathan Naylor G4KLX
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

#if !defined(MODENDEFINES_H)
#define  MODENDEFINES_H

#include <cstdint>

const uint8_t MODEN_RADIO_SYMBOL_LENGTH = 5U;      // At 24 kHz sample rate

const uint8_t MODEN_PREAMBLE_BYTE = 0x77U;

const uint8_t MODEN_SYMBOLS_PER_BYTE = 4U;

const uint8_t  MODEN_HEADER_PARITY_BYTES   = 2U;
const uint8_t  MODEN_HEADER_PARITY_SYMBOLS = MODEN_HEADER_PARITY_BYTES * MODEN_SYMBOLS_PER_BYTE;
const uint16_t MODEN_HEADER_PARITY_SAMPLES = MODEN_HEADER_PARITY_SYMBOLS * MODEN_RADIO_SYMBOL_LENGTH;

const uint8_t  MODEN_PAYLOAD_PARITY_BYTES   = 16U;
const uint8_t  MODEN_PAYLOAD_PARITY_SYMBOLS = MODEN_PAYLOAD_PARITY_BYTES * MODEN_SYMBOLS_PER_BYTE;
const uint16_t MODEN_PAYLOAD_PARITY_SAMPLES = MODEN_PAYLOAD_PARITY_SYMBOLS * MODEN_RADIO_SYMBOL_LENGTH;

const uint8_t  MODEN_CRC_LENGTH_BYTES     = 4U;
const uint8_t  MODEN_CRC_LENGTH_SYMBOLS   = MODEN_CRC_LENGTH_BYTES * MODEN_SYMBOLS_PER_BYTE;
const uint16_t MODEN_CRC_LENGTH_SAMPLES   = MODEN_CRC_LENGTH_SYMBOLS * MODEN_RADIO_SYMBOL_LENGTH;

const uint8_t  MODEN_SYNC_LENGTH_BYTES   = 6U;
const uint8_t  MODEN_SYNC_LENGTH_SYMBOLS = MODEN_SYNC_LENGTH_BYTES * MODEN_SYMBOLS_PER_BYTE;
const uint16_t MODEN_SYNC_LENGTH_SAMPLES = MODEN_SYNC_LENGTH_SYMBOLS * MODEN_RADIO_SYMBOL_LENGTH;
const uint8_t  MODEN_SYNC_BYTES[]        = {0x55U, 0xFDU, 0xDDU, 0x57U, 0xDFU, 0x7FU};

const uint8_t  MODEN_HEADER_LENGTH_BYTES   = 13U;
const uint8_t  MODEN_HEADER_LENGTH_SYMBOLS = MODEN_HEADER_LENGTH_BYTES * MODEN_SYMBOLS_PER_BYTE;
const uint16_t MODEN_HEADER_LENGTH_SAMPLES = MODEN_HEADER_LENGTH_SYMBOLS * MODEN_RADIO_SYMBOL_LENGTH;

const uint64_t MODEN_SYNC_BITS      = 0x000055FDDD57DF7FU;
const uint64_t MODEN_SYNC_BITS_MASK = 0x0000FFFFFFFFFFFFU;

// 5     5      F     D      D     D      5     7      D     F      7     F
// 01 01 01 01  11 11 11 01  11 01 11 01  01 01 01 11  11 01 11 11  01 11 11 11
// +3 +3 +3 +3  -3 -3 -3 +3  -3 +3 -3 +3  +3 +3 +3 -3  -3 +3 -3 -3  +3 -3 -3 -3

const int8_t MODEN_SYNC_SYMBOLS_VALUES[] = {+3, +3, +3, +3, -3, -3, -3, +3, -3, +3, -3, +3, +3, +3, +3, -3, -3, +3, -3, -3, +3, -3, -3, -3};

const uint32_t MODEN_SYNC_SYMBOLS      = 0x00F15E48U;
const uint32_t MODEN_SYNC_SYMBOLS_MASK = 0x00FFFFFFU;

#endif

